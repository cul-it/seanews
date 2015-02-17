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

/**
 * Find any expected issue for this publication and date - register the publication node
 * after a publication issue is being saved/updated we find the corresponding
 * expected_issue and put the nid of the publication in a computed field
 * @param  node $node publication node
 * @return node       unmodified publication node
 */
function seanews_action_node_register_expected_issue($node) {
  dsm($node);
  return array('node' => $node);
}
