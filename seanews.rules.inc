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
    'seanews_action_node_register_expected_issue' => array(
      'label' => t('Register uploaded issue with expected issue node'),
      'group' => t('Custom'),
      'parameter' => array(
        'node' => array('type' => 'node', 'label' => t('Publication Issue Node')),
      ),
    ),
    'seanews_action_node_lookup_issue' => array(
      'label' => t('Find publication issue for an expected issue node'),
      'group' => t('Custom'),
      'parameter' => array(
        'node' => array('type' => 'node', 'label' => t('Expected Issue Node')),
      ),
    ),
  );
}

/**
 * action code is in the .module since rules can't find it here???
 */


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

  dsm(array('found publication' => $found_nid));

  return array('node' => $node);
}

