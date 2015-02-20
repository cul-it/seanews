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
  return array(
    // 'seanews_action_node_register_expected_issue' => array(
    //   'label' => t('Register uploaded issue with expected issue node - obsolete'),
    //   'group' => t('Custom'),
    //   'parameter' => array(
    //     'node' => array('type' => 'node', 'label' => t('Publication Issue Node')),
    //   ),
    // ),
    // 'seanews_action_node_lookup_issue' => array(
    //   'label' => t('Find publication issue for an expected issue node - obsolete'),
    //   'group' => t('Custom'),
    //   'parameter' => array(
    //     'node' => array('type' => 'node', 'label' => t('Expected Issue Node')),
    //   ),
    // ),
    // 'seanews_action_node_lookup_publication_issue' => array(
    //   'label' => t('Find publication issue for a Publication Expected Issue node'),
    //   'group' => t('Custom'),
    //   'parameter' => array(
    //     'node' => array('type' => 'node', 'label' => t('Publication Expected Issue Node')),
    //   ),
    'seanews_action_node_render_view_text' => array(
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
    ),
  );
}


/**
 * Find any expected issue for this publication and date - register the publication node
 * after a publication issue is being saved/updated we find the corresponding
 * expected_issue and put the nid of the publication in a computed field
 * see seanews.rules.inc
 * @param  node $node publication node
 * @return node       unmodified publication node
 */
function seanews_action_node_register_expected_issue($node) {

  // find the publication of this publication_issue
  $field_items = field_get_items('node', $node, 'field_pub_issue_pub');
  $no_lang = array_pop($field_items);
  $publication= array_pop($no_lang);
  //dsm(array('publication',$publication));

  // find the date of this publication_issue
  $field_items = field_get_items('node', $node, 'field_pub_issue_date');
  $no_lang = array_pop($field_items);
  $pub_date = $no_lang['value'];
  //dsm(array('pub date', "$pub_date"));

  // lookup expected_issues that match this publisher and date
  $query = new EntityFieldQuery();

  $query->entityCondition('entity_type', 'node')
    ->entityCondition('bundle', 'expected_issue')
    ->propertyCondition('status', NODE_PUBLISHED)
    ->fieldCondition('field_expected_issue_date', 'value', $pub_date, '=')
    ->fieldCondition('field_expected_issue_publication', 'target_id', $publication, '=')
    ->range(0, 10)
    ->addMetaData('account', user_load(1)); // Run the query as user 1.

  $result = $query->execute();
  if (isset($result['node'])) {
    $expected_nids = array_keys($result['node']);
    foreach ($expected_nids as $nid) {
      $expected = node_load($nid);
      // set up the node reference from expected_issue to publication_issue
      $expected->field_expected_issue_reference[$expected->language][0]['nid'] = $node->nid;
      node_save($expected);
      /* for some reason this node_save hits this error:
      Notice: Undefined property: stdClass::$date_repeat_entity in date_repeat_entity_update_dates() (line 31 of /cul/data/aegir/platforms/dev-seanews-7.34-20150212-3/sites/all/modules/date_repeat_entity/includes/date_repeat_entity.update.inc). */
    }
  }

  return array('node' => $node);
}


/**
 * lookup publication_issue reference for the given expected_publication node
 * @param  node $node expected_publication node
 * @return node       modified node
 */
function seanews_action_node_lookup_issue($node) {

  // find the date of the expected_issue
  $field_items = field_get_items('node', $node, 'field_expected_issue_date');
  $no_lang = array_pop($field_items);
  $expected_date = $no_lang['value'];

  // find the publication of the expected_issue
  $field_items = field_get_items('node', $node, 'field_expected_issue_publication');
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

  $node->field_expected_issue_reference[$node->language][0]['nid'] = $found_nid;

  //module_load_include('inc', 'date_repeat_entity', 'includes/date_repeat_entity.utility');
  drupal_load('module', 'date_repeat_entity');

  dsm($node);
  //node_save($node);

  return array('node' => $node);
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
