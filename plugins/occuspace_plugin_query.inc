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
    $occuspaceAuthID = variable_get('occuspace_authID', "");
    if ($occuspaceAuthID == '') {
      return;
    }

    drupal_add_js(array('occuspace' => array('authID' => $occuspaceAuthID)), array
    ('type' => 'setting'));

    // Disabled Cache to ensure the data is current. could result in slower load times
    $ini = ini_set("soap.wsdl_cache_enabled", "0");

    // this obviously needs to change
    $url = $myOccuspace;

    $opts = array(
      'http' =>
        array(
          'method' => 'GET',
//          'header' => 'Authorization: ' . $occuspaceAuthID
            'header' => 'Authorization: Bearer ' . $occuspaceAuthID
        )
    );

    $context = stream_context_create($opts);
    $newfile = file_get_contents($url, false, $context);
    $jsondump = json_decode($newfile);
dsm($jsondump);


//    $parentGroup = $this->options['parentGroupName'];
//    $group = $this->options['groupName'];
////
//    $totalInUse = 0;
//    $totalAvailable = 0;
//    $totalOff = 0;
//    $totalTotal = 0;
//    $totalUnavailable = 0;


//    $totalInUse = $jsondump->InUse;
//    $totalAvailable = $jsondump->Available;
//    $totalOff = $jsondump->Offline;
//    $totalTotal = $jsondump->Total;
//    $totalUnavailable = 0;

//    //loop through results...we don't want what they don't ask for
//    //so if it's not what they entered in the query settings
//    //then we just continue on to the next one

      foreach ($jsondump->data as $entry){
          dsm($entry->parentId);
          if ($entry->parentId == null) {
              $entry->level = 'new';
          }
          dsm('level: ' . $entry->level);
      }


      foreach ($jsondump->data as $entry){
      $row = new stdClass();
      $row->idNumber = isset($entry->id) ? $entry->id : '';
      $row->name = isset($entry->name) ? $entry->name : '-';
      $row->parentId = isset($entry->parentId) ? $entry->parentId : '-';
      $row->capacity = isset($entry->capacity) ? $entry->capacity : '-';
      $row->earliestCount = isset($entry->earliestCount) ? $entry->earliestCount : '-';

      $view->result[] = $row;
    }
//    $row = new stdClass();
//    $row->totalInUse = $totalInUse;
//    $row->totalAvailable = $totalAvailable;
//    $row->totalOff = $totalOff;
//    $row->totalTotal = $totalTotal;
//    $row->totalUnavailable = $totalUnavailable;
//    $row->totalPercent = floor(($totalInUse/$totalTotal) * 100);
//    $view->result[] = $row;
  }
}