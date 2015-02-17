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
  );
}
