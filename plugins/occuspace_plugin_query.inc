<?php

/**
 * @file
 *   Views query plugin for occuspace stats.
 */
class occuspace_plugin_query extends views_plugin_query {

  function query($get_count = FALSE) {
  }

  /**
   * @param view $view
   * gets number live counts from occuspace
   */
  function execute(&$view) {
    //@todo add better error handling here
    //setting server variable
    $myOccuspace = variable_get('occuspace_server', "");
    if ($myOccuspace == '') {
      return;
    }
    $myOccuspaceBusy = variable_get('occuspace_server_busy', "");
    if ($myOccuspaceBusy == '') {
      return;
    }
    $occuspaceAuthID = variable_get('occuspace_authID', "");
    if ($occuspaceAuthID == '') {
      return;
    }

    drupal_add_js(array('occuspace' => array('authID' => $occuspaceAuthID)), array
    ('type' => 'setting'));

    // Disabled Cache to ensure the data is current. could result in slower load times
    $ini = ini_set("soap.wsdl_cache_enabled", "0");
//dsm($view);
    // set the filters based on admin configurations in the gui
    $filterNumber = ($view->display_handler->handlers['filter']['idNumber']->value['value']) ?? 0 ;
    $parentFilterNumber = ($view->display_handler->handlers['filter']['parentId']->value['value']) ?? 0 ;
    $chartChoice = $this->options['occuspace_chart_choice'];

    $url = $myOccuspace;
    $busyUrl = $myOccuspaceBusy;

    $opts = array(
      'http' =>
        array(
          'method' => 'GET',
            'header' => 'Authorization: Bearer ' . $occuspaceAuthID
        )
    );

    $context = stream_context_create($opts);
    $newfile = file_get_contents($url, false, $context);
    $jsondump = json_decode($newfile);

//    //loop through results...return only those that correspond to the filters in place in the gui
//      foreach ($jsondump->data as $entry){
//          if ($entry->parentId == null) {
//              $entry->level = 'parent';
//          }
//      }

      foreach ($jsondump->data as $entry){
//          dsm($filterNumber);
          if(($filterNumber>0) && ($entry->id != $filterNumber)) {continue;}
          if(($parentFilterNumber>0) && ($entry->parentId != $parentFilterNumber)) {continue;}

      $row = new stdClass();
      $row->idNumber = isset($entry->id) ? $entry->id : '';
      $row->name = isset($entry->name) ? $entry->name : '-';
      $row->parentId = isset($entry->parentId) ? $entry->parentId : '-';
      $row->capacity = isset($entry->capacity) ? $entry->capacity : '-';
      $row->earliestCount = isset($entry->earliestCount) ? $entry->earliestCount : '-';
      $row->level = isset($entry->level) ? $entry->level : '_';

      $busyOpts = array(
          'http' => array(
              'method' => 'GET',
              'header' => 'Authorization: Bearer ' . $occuspaceAuthID
          )
      );

      $contextBusy = stream_context_create($busyOpts);
      $newfileBusy = file_get_contents($busyUrl . '/' . $row->idNumber . '/now', false, $contextBusy);
      $jsondumpBusy = json_decode($newfileBusy);
      $row->count = isset($jsondumpBusy->data->count) ? $jsondumpBusy->data->count : '';
      $row->percentage = isset($jsondumpBusy->data->percentage) ? round($jsondumpBusy->data->percentage*100, 2) : '';
      $row->timestamp = isset($jsondumpBusy->data->timestamp) ? $jsondumpBusy->data->timestamp : '';
      $row->chartChoice = $chartChoice;

      $view->result[] = $row;
    }
  }

  //options for the query settings inside the view GUI
    function option_definition() {
        $options = parent::option_definition();
        $options['occuspace_chart_choice'] = array('default' => 'noGauge');
        $options['occuspace_data_format_type'] = array('default' => 'people');
        $options['occuspace_pieHole'] = array('default' => 65);
        $options['occuspace_slice_text_color'] = array('default' => 'white');
        $options['occuspace_available_label'] = array('default' => 'Available');
        $options['occuspace_count_label'] = array('default' => 'People');
        $options['occuspace_green_color'] = array('default' => '#00B239');
        $options['occuspace_yellow_color'] = array('default' => '#FFC300');
        $options['occuspace_red_color'] = array('default' => '#B20000');
        $options['occuspace_font_size'] = array('default' => 17);
        $options['occuspace_color_change'] = array('default' => 'true');
        return $options;
    }
    //Creating the fields for the query settings in the view GUI
    function options_form(&$form, &$form_state) {
        parent::options_form($form, $form_state);
        $form['occuspace_data_format_type'] = array(
            '#title' => t('Data Format'),
            '#type' => 'radios',
            '#description' => 'How would you like the data formatted?',
            '#default_value' => $this->options['occuspace_data_format_type'],
            '#options' => array(
                'people' => t('Number of people'),
                'percentage' => t('Percentage of Occupancy'),
            ),
        );

        $form['occuspace_available_label'] = array(
            '#title' => t('Available label'),
            '#type' => 'textfield',
            '#description' => 'The label for the chart indicating the number of people that can still enter the zone.',
            '#default_value' => $this->options['occuspace_available_label'],
        );

        $form['occuspace_font_size'] = array(
            '#title' => t('Font Size'),
            '#type' => 'textfield',
            '#description' => 'Enter the font size (px) of the chart text.',
            '#default_value' => $this->options['occuspace_font_size'],
        );

        $form['occuspace_chart_choice'] = array(
            '#title' => t('Pie - Chart style'),
            '#type' => 'radios',
            '#description' => 'Choose between a speedometer type gauge pie chart or a half donut type pie chart. *Pie chart only',
            '#default_value' => $this->options['occuspace_chart_choice'],
            '#options' => array(
                'yesGauge' => t('Speedomoter/Gauge'),
                'noGauge' => t('Half Donut'),
            ),
        );

        $form['occuspace_pieHole'] = array(
            '#title' => t('Pie - Pie Hole'),
            '#type' => 'textfield',
            '#description' => 'The blank space in the middle of the pie chart. Enter a number between 0 and 1. To shut your piehole, enter 0. *Pie chart only',
            '#default_value' => $this->options['occuspace_pieHole'],
            '#size' => 3,
        );

        $form['occuspace_slice_text_color'] = array(
            '#title' => t('Pie - Slice Text Color'),
            '#type' => 'textfield',
            '#description' => 'The color (hex or color name) for the text on the pie slices. *Pie chart only',
            '#default_value' => $this->options['occuspace_slice_text_color'],
        );

        $form['occuspace_green_color'] = array(
            '#title' => t('Gauge - first section color'),
            '#type' => 'textfield',
            '#description' => 'The color for the first section of the chart (hex or color name). Default is green. *Gauge type only',
            '#default_value' => $this->options['occuspace_green_color'],
        );

        $form['occuspace_yellow_color'] = array(
            '#title' => t('Gauge - second section color'),
            '#type' => 'textfield',
            '#description' => 'The color for the second section of the chart (hex or color name). Default is yellow. *Gauge type only',
            '#default_value' => $this->options['occuspace_yellow_color'],
        );

        $form['occuspace_red_color'] = array(
            '#title' => t('Gauge - last section color'),
            '#type' => 'textfield',
            '#description' => 'The color for the last section of the chart (hex or color name). Default is red. *Gauge type only',
            '#default_value' => $this->options['occuspace_red_color'],
        );

        $form['occuspace_color_change'] = array(
            '#title' => t('Bar - Color Changing based on percentage?'),
            '#type' => 'radios',
            '#description' => 'Do you want the bar to be green/yellow/red based on percentage of capacity? *Bar type only',
            '#default_value' => $this->options['occuspace_color_change'],
            '#options' => array(
                'true' => t('Yes, bar will auto-change color)'),
                'false' => t('No, bar will be set to custom color in field settings for entire bar)'),
            ),
        );
    }
}