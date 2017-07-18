<?php

/**
 * @file
 *
 * user pages.
 */

function exact_target_simple_subscribe_modal_page($js, $delta) {
  ctools_include('modal');
  ctools_include('ajax');


  $config = variable_get('exact_target_simple_subscribe_blocks', array());

  if (empty($delta) || empty($config[$delta])) {
    // we don't have a configuration
    drupal_not_found();
    return;
  }
  else {
    $config = $config[$delta];
  }

  $form_state = array(
    'title' => check_plain($config['modal_title']),
    'ajax' => $js,
    'delta' => $delta,
    'config' => $config,
  );
  $output = ctools_modal_form_wrapper('exact_target_simple_subscribe_modal_form', $form_state);
  // There are some possible states after calling the form wrapper:
  // 1) We are not using $js and the form has been executed.
  // 2) We are using $js and the form was successfully submitted and
  //    we need to dismiss the modal.
  // Most other states are handled automatically unless you set flags in
  // $form_state to avoid handling them, so we only deal with those two
  // states.
  if ($form_state['executed'] && $js) {
    $commands = array();
    $commands[] = ctools_modal_command_display(t('Subscription Success'), $form_state['message']);
    // In typical usage you will do something else here, such as update a
    // div with HTML or redirect the page based upon the results of the modal
    // form.
    print ajax_render($commands);
    return;
  }

  // Otherwise, just return the output.
  if ($js) {
    print ajax_render($output);
    return;
  }
  else {
    drupal_set_title(check_plain($config['modal_title']));
    return $output;
  }
}

function exact_target_simple_subscribe_modal_form($form, &$form_state) {
  $delta = $form_state['delta'];
  $config = $form_state['config'];
  if (empty($delta) || empty($config)) {
    // we don't have a configuration
    drupal_not_found();
    return;
  }

  if (!empty($config['form_body']['value'])) {
    $form['body'] = array(
      '#markup' => check_markup($config['form_body']['value'], $config['form_body']['format']),
    );
  }

  $subscribe_lists = array();
  foreach ($config['lists'] AS $key => $value) {
    if ($value) {
      $list = exact_target_api_get_list_info($key);
      $subscribe_lists[$key] = $list->ListName;
    }
  }

  $form['subscribe_lists'] = array(
    '#type' => 'checkboxes',
    '#title' => t('To which lists do you wish to subscribe?'),
    '#options' => $subscribe_lists,
  );
  if (count($subscribe_lists) == 1) {
    // there is only one option so we will check it and not allow it to be
    // unchecked
    $key = array_keys($subscribe_lists);
    $key = $key[0];
    $form['subscribe_lists']['#default_value'] = array($key => $key);
    $form['subscribe_lists']['#disabled'] = TRUE;
  }
  else if (count($subscribe_lists) == 0) {
    drupal_not_found();
    return;
  }

  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email Address'),
    '#description' => t('We will use this email address to send you periodic messages.'),
    '#required' => TRUE,
  );

  $form['subscribe'] = array(
    '#type' => 'submit',
    '#value' => t('Subscribe'),
  );
  return $form;
}

function exact_target_simple_subscribe_modal_form_validate($form, &$form_state) {
  if (!valid_email_address($form_state['values']['email'])) {
    form_set_error('email', t('Not a valid email address'));
  }
  $subscribe_count = 0;
  foreach ($form_state['values']['subscribe_lists'] AS $value) {
    if ($value) {
      $subscribe_count++;
    }
  }
  if (!$subscribe_count) {
    form_set_error('subscribe_lists', t('No mailing lists selected.'));
  }
}

function exact_target_simple_subscribe_modal_form_submit($form, &$form_state) {
  $lists = array();
  foreach ($form_state['values']['subscribe_lists'] AS $value) {
    if ($value) {
      $lists[] = $value;
    }
  }
  $result = exact_target_api_add_subscriber($lists, $form_state['values']['email']);

  if ($result->status) {
    $form_state['message'] = format_plural(
      count($lists),
      '%email has been subscribed to the selected list.',
      '%email has been subscribed to the selected lists.',
      array('%email' => $form_state['values']['email'])
    );
    if (!$form_state['ajax']) {
      drupal_set_message($message);
    }
  }
}
