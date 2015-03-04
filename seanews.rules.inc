<?php
// $Id$

/**
 * @file
 * Southeast Asia Newspaper Archive Tools for Rules
 */

/**
 * Implementation of hook_rules_action_info().
 * @ingroup rules
 */
function seanews_rules_action_info() {
  $actions['seanews_action_node_lookup_publication_issue'] = array(
      'label' => t('Find publication issue for a Publication Expected Issue node'),
      'group' => t('Custom'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Publication Expected Issue Node'),
          ),
        ),
      );
  $actions['seanews_action_node_save_publication_issue_update_expected'] = array(
      'label' => t('Update corresponding expected issue'),
      'group' => t('Custom'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Publication Issue Node'),
          ),
        ),
      );
  $actions['seanews_action_node_save_publication_issue_move_file'] = array(
      'label' => t('Move the uploaded file into publisher/year private directory.'),
      'group' => t('Custom'),
      'parameter' => array(
        'node' => array(
          'type' => 'node',
          'label' => t('Publication Issue Node'),
          ),
        ),
      );
  $actions['seanews_action_node_render_view_text'] = array(
      'label' => t('Render the output of a view as text'),
      'group' => t('Custom'),
      'parameter' => array(
        'viewname' => array('type' => 'text', 'label' => t('Machine name of view')),
        'display' => array('type' => 'text', 'label' => t('Display name within view')),
        'argument1' => array('type' => 'text', 'label' => t('View argument 1'), 'optional' => TRUE),
        'argument2' => array('type' => 'text', 'label' => t('View argument 2'), 'optional' => TRUE),
        'argument3' => array('type' => 'text', 'label' => t('View argument 3'), 'optional' => TRUE),
      ),
      'provides' => array(
        'variable_added' => array(
          'type' => 'text',
          'label' => t('Added variable'),
          ),
        ),
      );
  return $actions;
}

/**
 * lookup field_publication_reference for the given publication_expected_issue node
 * @param  node $node publication_expected_issue node
 * @return node       modified node
 */
function seanews_action_node_lookup_publication_issue($node) {

  // find the date of the publication_expected_issue
  $field_items = field_get_items('node', $node, 'field_pub_expected_issue_date');
  $no_lang = array_pop($field_items);
  $expected_date = $no_lang['value'];

  // find the publication of the publication_expected_issue
  $field_items = field_get_items('node', $node, 'field_publication_reference');
  $no_lang = array_pop($field_items);
  $expected_publication= array_pop($no_lang);

  $query = new EntityFieldQuery();

  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'publication_issue')
    ->propertyCondition('status', NODE_PUBLISHED)
    ->fieldCondition('field_pub_issue_date', 'value', $expected_date, '=')
    ->fieldCondition('field_pub_issue_pub', 'target_id', $expected_publication, '=')
    ->fieldCondition('field_pub_issue_document', 'fid', 'NULL', '!=')
    ->range(0, 10)
    ->addMetaData('account', user_load(1)); // Run the query as user 1.

  $result = $query->execute();

  if (isset($result['node'])) {
    $publication_nids = array_keys($result['node']);
    $found_nid = array_pop($publication_nids);
  }
  else {
    $found_nid = 0;
  }

  if ($node->field_issue_reference[$node->language][0]['nid'] != $found_nid) {
    // update the node with the publicaton issue reference
    $node->field_issue_reference[$node->language][0]['nid'] != $found_nid;
    if ($found_nid != 0) dsm($node);
    //node_save($node);
  }

  return array('node' => $node);
}

function seanews_action_node_render_view_text($viewname, $display, $argument1 = NULL, $argument2 = NULL, $argument3 = NULL) {
  $view = views_get_view($viewname , TRUE);
  $arg = array($argument1, $argument2, $argument3);
  $views_result = $view->preview($display, $arg);
  $output = drupal_html_to_text($views_result);
  return array('variable_added' => $output);
}

/**
 * connect expected issue to recently uploaded publication issue
 * @param  object $node publication issue node
 */
function seanews_action_node_save_publication_issue_update_expected($node) {
 dsm('seanews_action_node_save_publication_issue_update_expected');
}

/**
 * move uploaded issue file to private://<publication>/<year>/foo.ext
 * @param  object $node publication issue node
 * @return [type] [description]
 */
function seanews_action_node_save_publication_issue_move_file($node) {
  dsm('seanews_action_node_save_publication_issue_move_file');
  try {
    $wrapper = entity_metadata_wrapper('node', $node);
    dsm($wrapper->getPropertyInfo());
    $issue_datetime = $wrapper->field_pub_issue_date->value();
    $issue_year = format_date($issue_datetime, 'custom', 'Y');
    $pub_path = $wrapper->field_pub_issue_pub->field_pub_title_r_cul_path->value();
    dsm(array($issue_year,$pub_path));
    $file = $wrapper->field_pub_issue_document->file->value();
    $fid = $wrapper->field_pub_issue_document->file->fid->value();
    $original_name = $file->filename; //$wrapper->field_pub_issue_document->file->filename->value();
    dsm(array($file,$fid,$original_name));
    $destination = 'private://issues/' . $pub_path . '/' . $issue_year . '/';
    if(file_prepare_directory($destination, FILE_CREATE_DIRECTORY)) {
      $new_file = file_copy($file_object, $destination .  $original_name, FILE_EXISTS_RENAME);
      if ($new_file) {
         $new_file->status = FILE_STATUS_PERMANENT;
         drupal_chmod($new_file->uri, 0775);
         $new_file = file_save($new_file);
         $wrapper->field_pub_issue_document->file->set($new_file);
      }
     dsm($new_file);
    }
  }
  catch (EntityMetadataWrapperException $exc) {
      watchdog(
        'seanews',
        'See '  . __FUNCTION__ . '() <pre>' .  $exc->getTraceAsString() . '</pre>',
         NULL, WATCHDOG_ERROR
      );
  }
  return array('node' => $node);
}
