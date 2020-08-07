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
        return $options;
    }
    //Creating the fields for the query settings in the view GUI
    function options_form(&$form, &$form_state) {
        parent::options_form($form, $form_state);
        $form['occuspace_chart_choice'] = array(
            '#title' => t('Gauge?'),
            '#type' => 'radios',
            '#description' => 'Would you like to be really cool and make this a Gauge chart type?',
            '#default_value' => $this->options['occuspace_chart_choice'],
            '#options' => array(
                'yesGauge' => t('Yes Gauge!'),
                'noGauge' => t('No Gauge.'),
            ),
        );
    }
}