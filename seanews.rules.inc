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
      'label' => t('Move the uploaded file into publisher/year/month private directory.'),
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
  //dsm('seanews_action_node_save_publication_issue_update_expected');
  try {
    if ($node->type != 'publication_issue') {
      throw new Exception("Wrong node type", 1);
    }
    $wrapper = entity_metadata_wrapper('node', $node);
    //dsm($wrapper->getPropertyInfo());
    $issue_datetime = $wrapper->field_pub_issue_date->value();
    $issue_date = date('Y-m-d H:i:s', $issue_datetime);
    $issue_publicaton = $wrapper->field_pub_issue_pub->nid->value();
    $issue_nid = $wrapper->nid->value();

    //dsm($issue_datetime);
    //dsm($issue_date);
    //dsm($issue_publicaton);
    //dsm($issue_nid);

    $query = new EntityFieldQuery();

    $query->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', 'publication_expected_issue')
      ->propertyCondition('status', NODE_PUBLISHED)
      ->fieldCondition('field_pub_expected_issue_date', 'value', $issue_date, '=')
      ->fieldCondition('field_publication_ent_reference', 'target_id', $issue_publicaton, '=')
      ->range(0, 10)
      ->addMetaData('account', user_load(1)); // Run the query as user 1.

    $result = $query->execute();

    if (isset($result['node'])) {
      $expected_nids = array_keys($result['node']);
      //dsm($expected_nids);
      foreach ($expected_nids as $found_nid) {
        // update expected issue with a reference to this issue_publication
        // by re-saving the node to recalculate it's computed field
        $node = node_load($found_nid);

        /*
        Prevent:
        Notice: Undefined property: stdClass::$date_repeat_entity in date_repeat_entity_update_dates() (line 31 of /cul/data/aegir/platforms/dev-seanews-7.34-20150304-2/sites/all/modules/date_repeat_entity/includes/date_repeat_entity.update.inc).

        during node_save below
        (means only update the current node's repeating dates)
        */
        $node->date_repeat_entity['scope'] = 'current';

        node_save($node);
        //dsm("set issue for expected issue $found_nid");
      }
    }
    else {
      //dsm("no expected issues for $issue_nid");
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

function seanews_s3_folder($str) {
  $bad = array(' ', '_', '.-', '-.', '\\', '/');
  $aok = array('-', '-', '-',  '-',  '-',  '-');
  $str = str_replace($bad, $aok, $str);
  $str = preg_replace('/[^a-z0-9\-\.]/', '', $str);
  if (strlen($str) < 3 || strlen($str) > 63) {
    throw new Exception("S3 string length exception", 1);
  }
  if (preg_match('/[a-z0-9]/', substr($str, 0, 1)) != 1) {
    $str = 'a' . $str;
  }

}

/**
 * move uploaded issue file to private://<publication>/<year>/<month>/foo.ext
 * @param  object $node publication issue node
 * @return [type] [description]
 */
function seanews_action_node_save_publication_issue_move_file($node) {
  return array('node' => $node);
  try {
    $wrapper = entity_metadata_wrapper('node', $node);
    $issue_datetime = $wrapper->field_pub_issue_date->value();
    $issue_year = format_date($issue_datetime, 'custom', 'Y');
    $issue_month = format_date($issue_datetime, 'custom', 'm');
    $pub_path = $wrapper->field_pub_issue_pub->field_pub_title_r_cul_path->value();

    // loop over multiple uploaded files
    $new_file_found = FALSE;
    foreach ($wrapper->field_pub_issue_document->getIterator() as $delta => $item_wrapper) {
      $file = $item_wrapper->file->value();
      $fid = $item_wrapper->file->fid->value();
      $original_name = $file->filename;

      /*
      file_prepare_directory only sets permission on the last directory in the path, so
      step through each level of the path in case one needs to be created
      $destination = "private://issues/$pub_path/$issue_year/$issue_month/";
      */
      $steps = array('issues', $pub_path, $issue_year, $issue_month);
      $destination = "private://";
      foreach ($steps as $directory) {
        $destination .= "$directory/";
        // if (file_prepare_directory($destination, FILE_CREATE_DIRECTORY) === FALSE) {
        //   throw new Exception("Unable to create path: $destination", 1);
        // }
        // if (mkdir($destination, 0775, true) === false) {
        //   throw new Exception("Unable to create path mkdir: $destination", 1);
        // }
        // purposfully not checking error!!
        file_prepare_directory($destination, FILE_CREATE_DIRECTORY);
        $temp = drupal_tempnam($destination, 'placeholder');
        //dpm($temp, 'temp name');
      }
      dpm("file_move to $destination $original_name");
      $new_file = file_move($file, $destination .  $original_name, FILE_EXISTS_RENAME);
      if ($new_file) {
         $new_file->status = FILE_STATUS_PERMANENT;
         drupal_chmod($new_file->uri, 0775);
         dpm("file_save $new_file->uri");
         $new_file = file_save($new_file);
         dpm("wrapper file set");
         $item_wrapper->file->set($new_file);
         $new_file_found = TRUE;
      }
      else {
        throw new Exception("Unable to move file to $destination", 1);
      }
    }
    if ($new_file_found) {
      dpm("wrapper save");
      $wrapper->save();
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

