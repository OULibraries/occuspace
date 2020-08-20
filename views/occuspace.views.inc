<?php
//drupal_add_js(drupal_get_path('module', 'http://labstats.ou.edu/api/public/GetPublicApiData/1001') . '/occuspace.js');
//drupal_add_css('https://labstats.ou.edu/Content/MapViewer.css', 'external');
//drupal_add_js('https://cdn.jsdelivr.net/excanvas/r3/excanvas.compiled.js', 'external');
//drupal_add_js('https://labstats.ou.edu/Scripts/MapViewer.js', 'external');
drupal_add_js('https://code.highcharts.com/highcharts.js', 'external');

/**
 * Implements hook_views_plugins().
 */
function occuspace_views_plugins() {
  $plugin = array();
  $plugin['query']['occuspace_plugin_query'] = array(
    'title' => t('Occuspace Query'),
    'help' => t('Occuspace query object.'),
    'handler' => 'occuspace_plugin_query',
  );
  return $plugin;
}

/**
 * Implements hook_views_data().
 *
 * The fields that the user gets to choose from to display
 */
function occuspace_views_data() {
  $data = array();

  //Base data
  $data['occuspace']['table']['group'] = t('Occuspace Stats');
  $data['occuspace']['table']['base'] = array(
    'title' => t('Occuspace Stats'),
    'help' => t('Query Occuspace Stats'),
    'query class' => 'occuspace_plugin_query'
  );

  //Fields
  $data['occuspace']['idNumber'] = array(
    'title' => t('ID Number'),
    'help' => t('The ID Number of this zone'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
    'filter' => array(
        'handler' => 'occuspace_handler_filter_id',
    ),
  );
  $data['occuspace']['name'] = array(
    'title' => t('Zone Name'),
    'help' => t('The Name assigned to this zone'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  $data['occuspace']['parentId'] = array(
    'title' => t('Parent ID'),
    'help' => t('The ID of the Parent zone, or the zone that contains this zone'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
      'filter' => array(
          'handler' => 'occuspace_handler_filter_id',
      ),
  );
  $data['occuspace']['capacity'] = array(
    'title' => t('Capacity'),
    'help' => t('The total number of people that should be allowed into this area at one time.'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  $data['occuspace']['earliestCount'] = array(
    'title' => t('Earliest Count'),
    'help' => t('The date of the first time this zone was counted.'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  $data['occuspace']['level'] = array(
    'title' => t('Level'),
    'help' => t('The level within the hierarchy. (Parent, Child, etc)'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  $data['occuspace']['count'] = array(
    'title' => t('Count'),
    'help' => t('The number of people detected right now.'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  $data['occuspace']['percentage'] = array(
    'title' => t('Percentage'),
    'help' => t('The percentage of occupancy currently....um...occupied'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  $data['occuspace']['timestamp'] = array(
    'title' => t('Timestamp'),
    'help' => t('The time (as a string) of the last count'),
    'field' => array(
      'handler' => 'occuspace_handler_field',
    ),
  );
  return $data;
}