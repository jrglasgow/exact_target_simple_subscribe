<?php

/**
 * @file
 *
 * Administrator hooks/callbacks
 */

 /**
  * Implenents hook_block_info().
  */
function _exact_target_simple_subscribe_block_info() {
  $etss_blocks_config = variable_get('exact_target_simple_subscribe_blocks', array());
  $blocks = array();
  if (empty($etss_blocks_config)) {
    return array();
  }
  $lists = exact_target_api_get_lists();
  foreach ($etss_blocks_config AS $delta => $config) {
    $enabled_lists = array();
    foreach ($config['lists'] AS $key => $value) {
      if ($value) {
        $enabled_lists[] = '"' . $lists[$key]->ListName . '"';
      }
    }
    $blocks[$delta] = array(
      'info' => t('Exact Target Simple Subscribe block #@delta using lists @lists', array('@delta' => $delta, '@lists' => implode(', ', $enabled_lists))),
      'cache' => DRUPAL_NO_CACHE,
    );
  }
  return $blocks;
}

/**
 * Implements hook_block_configure().
 */
function _exact_target_simple_subscribe_block_configure($delta) {
  $form = array();
  $lists = exact_target_api_get_lists();
  $form['etss'] = array(
    '#tree' => TRUE,
  );


  $form['info'] = array(
    '#type' => 'value',
    '#value' => t('Exact Target Simple Subscribe block #@delta', array('@delta' => $delta)),
  );
  $config = variable_get('exact_target_simple_subscribe_blocks', array());
  $config = $config[$delta];
  
  $form['etss']['block_body'] = array(
    '#type' => 'text_format',
    '#title' => t('Block Body'),
    '#description' => t('Text that will appear in the block above the "Subscribe" button.'),
    '#default_value' => $config['block_body']['value'],
    '#format' => $config['block_body']['format'],
  );

  $form ['etss']['block_button_text'] = array(
    '#type' => 'textfield',
    '#title' => t('"Subscribe" button text'),
    '#description' => t('The actual text on the subscribe button that appears in the block.'),
    '#default_value' => $config['block_button_text'] ?: t('Subscribe'),
  );

  $form ['etss']['block_button_title'] = array(
    '#type' => 'textfield',
    '#title' => t('"Subscribe" button TITLE text.'),
    '#description' => t('Text to appear in the TITLE attribute for the button in the block.'),
    '#default_value' => $config['block_button_title'] ?: t('Subscribe to this mailing list.'),
  );

  $form['etss']['modal_title'] = array(
    '#type' => 'textfield',
    '#title' => t('Form Title'),
    '#description' => t('The title of the page or modal window which contains the subscribe form.'),
    '#default_value' => $config['modal_title'] ?: t('Subscribe to mailing list.'),
  );

  $form['etss']['form_body'] = array(
    '#type' => 'text_format',
    '#title' => t('Form Body'),
    '#description' => t('Text that will appear in the modal window that pops up above the form.'),
    '#default_value' => $config['form_body']['value'],
    '#format' => $config['form_body']['format'],
  );

  $form['etss']['lists'] = array(
    '#type' => 'fieldset',
    '#title' => t('Lists'),
    '#description' => t('Which list(s) should the user be allowed to subscribe to?'),
    '#tree' => TRUE,
  );
  $form['etss']['modal_style'] = array(
    '#type' => 'fieldset',
    '#title' => t('Modal Style'),
    '#description' => t('Configure the style of the modal window.'),
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
  );
  $form['etss']['modal_style']['style'] = array(
    '#type' => 'radios',
    '#title' => 'Style',
    '#description' => t('Choose a pre-defined style.'),
    '#options' => array(
      'etss-subscribe-style' => t('Exact Target Simple Subscribe'),
      'other' => t('Other'),
    ),
    '#default_value' => $config['modal_style']['style'] ?: 'etss-subscribe-style',
  );

  $form['etss']['modal_style']['other'] = array(
    '#type' => 'textfield',
    '#title' => t('Other'),
    '#description' => t('Enter in the style name of a cTools modal style defined in another module or the theme.'),
    '#default_value' => $config['modal_style']['other'] ?: '',
    '#states' => array(
      'visible' => array(
        ':input[name="etss[modal_style][style]"]' => array('value' => 'other'),
      ),
    ),
  );

  $form['etss']['modal_style']['type'] = array(
    '#type' => 'radios',
    '#title' => t('Type'),
    '#description' => t('How is modal window size defined? Scale is a precentage of the frame, while fixed is pixels.'),
    '#default_value' => $config['modal_style']['type'] ?: 'scale',
    '#options' => array(
      'fixed' => 'fixed',
      'scale' => 'scale',
    ),
    '#states' => array(
      'invisible' => array(
        ':input[name="etss[modal_style][style]"]' => array('value' => 'other'),
      ),
    ),
  );

  $form['etss']['modal_style']['width'] = array(
    '#type' => 'textfield',
    '#title' => t('Width'),
    '#description' => t('The width of the modal window.'),
    '#default_value' => $config['modal_style']['width'] ?: '75',
    '#states' => array(
      'invisible' => array(
        ':input[name="etss[modal_style][style]"]' => array('value' => 'other'),
      ),
    ),
    '#size' => 5,
  );

  $form['etss']['modal_style']['height'] = array(
    '#type' => 'textfield',
    '#title' => t('Height'),
    '#description' => t('The height of the modal window.'),
    '#default_value' => $config['modal_style']['height'] ?: '50',
    '#states' => array(
      'invisible' => array(
        ':input[name="etss[modal_style][style]"]' => array('value' => 'other'),
      ),
    ),
    '#size' => 5,
  );

  foreach ($lists AS $list) {
    $form['etss']['lists'][$list->ID] = array(
      '#type' => 'checkbox',
      '#title' => $list->ListName,
      '#description' => $list->Description,
      '#default_value' => $config['lists'][$list->ID],
    );
  }

  return $form;
}

/**
 * Implements hook_block_save().
 */
function _exact_target_simple_subscribe_block_save($delta, $edit) {
  $etss_blocks_config = variable_get('exact_target_simple_subscribe_blocks', array());
  $etss_blocks_config[$delta] = $edit['etss'];
  variable_set('exact_target_simple_subscribe_blocks', $etss_blocks_config);
}

/**
 * Implements hook_block_view().
 */
function _exact_target_simple_subscribe_block_view($delta) {
  $config = variable_get('exact_target_simple_subscribe_blocks', array());
  if (empty($config[$delta])) {
    return;
  }
  else {
    $config = $config[$delta];
  }

  // Include the CTools tools that we need.
  ctools_include('ajax');
  ctools_include('modal');

  // Add CTools' javascript to the page.
  ctools_modal_add_js();

  $block_content = check_markup($config['block_body']['value'], $config['block_body']['format']);
  $modal_button .= ctools_modal_text_button(check_plain($config['block_button_text']), 'ajax/etss_subscribe/nojs/' . $delta, $config['block_button_title'], 'ctools-modal-' . $config['modal_style']['style']);

  $content=<<<CONTENT
  <div class="block-content">$block_content</div>
  <div class="modal-button">$modal_button</div>
CONTENT;

  $return =  array(
    'subject' => $config['subject'] ?: '',
    'content' => $content,
  );

  if ($config['modal_style']['style'] != 'other') {
    if ($config['modal_style']['type'] == 'scale') {
      $width = $config['modal_style']['width']/100;
      $height = $config['modal_style']['height']/100;
    }
    else {
      $width = (int) $config['modal_style']['width'];
      $height = (int) $config['modal_style']['height'];
    }
    drupal_add_js(
      array(
        'etss-subscribe-style' => array(
          'modalSize' => array(
            'type' => $config['modal_style']['type'] ?: 'scale',
            'width' => $width ?: .5,
            'height' => $height ?: .7,
          ),
        ),
      ),
      'setting'
    );
  }

  return $return;
}


function exact_target_simple_subscribe_add_block_form($form, &$form_state, $theme_key) {
  $block_config = variable_get('exact_target_simple_subscribe_blocks', array());
  $keys = array_keys($block_config);
  $delta = empty($keys) ? 1 : max($keys) + 1;
  drupal_goto('admin/structure/block/manage/exact_target_simple_subscribe/' . $delta . '/configure');
}

/**
 * delete a single block
 */
function exact_target_simple_subscribe_delete_form($form, &$form_state, $delta) {
  $blocks = _exact_target_simple_subscribe_block_info();

  return confirm_form($form, t('Are you sure you want to delete the block `@title`?', array('@title' => $blocks[$delta]['info'])));
}

function exact_target_simple_subscribe_delete_form_submit(&$form, &$form_state) {
  $delta = $form_state['build_info']['args'][0];
  $block_config = variable_get('exact_target_simple_subscribe_blocks', array());
  unset($block_config[$delta]);
  variable_set('exact_target_simple_subscribe_blocks', $block_config);
  drupal_set_message(t('Exact Target Simple Subscribe block #@delta has been deleted.', array('@delta' => $delta)));
  $form_state['redirect'] = 'admin/structure/block';
}
