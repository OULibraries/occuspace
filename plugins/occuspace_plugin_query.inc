<?php
    /**
     * @file
     *   Views query plugin for occuspace stats.
     */
    class occuspace_plugin_query extends views_plugin_query {
        function query($get_count = FALSE) {}

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

            $occuspace_use_cookie = variable_get('occuspace_use_cookie', "true");
            $occuspace_cookie_version = variable_get('occuspace_cookie_version', "1");
            $occuspace_busy_data_cookie_time = variable_get('occuspace_busy_data_cookie_time', "5");

            drupal_add_js(['occuspace' => ['authID' => $occuspaceAuthID]], ['type' => 'setting']);

            // Disabled Cache to ensure the data is current. could result in slower load times
            $ini = ini_set("soap.wsdl_cache_enabled", "0");

            // set the filters based on admin configurations in the gui
            $filterNumber = ($view->display_handler->handlers['filter']['idNumber']->value['value']) ?? 0;
            $parentFilterNumber = ($view->display_handler->handlers['filter']['parentId']->value['value']) ?? 0;
            $chartChoice = $this->options['occuspace_chart_choice'];
            $url = $myOccuspace;
            $busyUrl = $myOccuspaceBusy;

            //check if cookie exists in user's browser to eliminate the first api call for spaces data.
            $save_new_cookie = true;
            $make_api_call = false;

            // get cookie
            $cookie_exists = isset($_COOKIE["Drupal_visitor_occuspace_spaces"]);

            // admin set to not use cookie anymore. Delete cookie
            if ($occuspace_use_cookie === 'false') {
                if ($cookie_exists === true) { //delete existing cookie
                    user_cookie_delete('occuspace_spaces');
                }

                $make_api_call = true;
                $save_new_cookie = false;
            } else {
                if ($cookie_exists === true || $cookie_exists === '1') { // cookie exists, check version
                    $cookie_data = $_COOKIE["Drupal_visitor_occuspace_spaces"];
                    $json_data = json_decode($cookie_data);
                    $saved_cookie_version = $json_data->version;

                    if ($saved_cookie_version == $occuspace_cookie_version) {
                        $save_new_cookie = false;
                        $jsondump = $json_data;
                        watchdog('occuspace_data', 'Occuspace space data used from cookie', []);
                    } else {
                        $make_api_call = true;
                        $save_new_cookie = true;
                        user_cookie_delete('occuspace_spaces');
                    }
                } else { // cookie does not exists, need to create it and save it.
                    $make_api_call = true;
                    $save_new_cookie = true;
                }
            }

            if ($make_api_call === true) { // based on settings, api call for spaces needs to be ran
                $opts = [
                    'http' => [
                        'method' => 'GET',
                        'header' => 'Authorization: Bearer ' . $occuspaceAuthID,
                    ],
                ];
                $context = stream_context_create($opts);
                $newfile = file_get_contents($url, FALSE, $context);
                watchdog('occuspace_data', 'Occuspace space data fetched using api. Url - %url', ['%url'=>$url]);
                $jsondump = json_decode($newfile);
            }

            if ($save_new_cookie === true) { // based on settings, cookie needs saved for this data to eliminate api call until version changes in settings
                $jsondump->version = $occuspace_cookie_version;
                $newData = json_encode($jsondump);
                user_cookie_save(['occuspace_spaces' => $newData]);
            }

            foreach ($jsondump->data as $entry) {
                if (($filterNumber > 0) && ($entry->id != $filterNumber)) {
                    continue;
                }

                if (($parentFilterNumber > 0) && ($entry->parentId != $parentFilterNumber)) {
                    continue;
                }

                $row = new stdClass();
                $row->idNumber = isset($entry->id) ? $entry->id : '';
                $row->name = isset($entry->name) ? $entry->name : '-';
                $row->parentId = isset($entry->parentId) ? $entry->parentId : '-';
                $row->capacity = isset($entry->capacity) ? $entry->capacity : '-';
                $row->earliestCount = isset($entry->earliestCount) ? $entry->earliestCount : '-';
                $row->level = isset($entry->level) ? $entry->level : '_';

                $cookie_name = 'occuspace_busy_data_'.$row->idNumber;
                // get cookie
                $busy_cookie_exists = isset($_COOKIE["Drupal_visitor_".$cookie_name]);

                //check if cookie exists in user's browser to eliminate the first api call for spaces data.
                $save_new_busy_cookie = false;
                $make_busy_api_call = false;

                // admin set to not use cookie anymore. Delete cookie
                if ($occuspace_use_cookie === 'false') {
                  $make_busy_api_call = true;
                  if ($busy_cookie_exists === true) {
                    user_cookie_delete($cookie_name);
                  }
                }

                if ($occuspace_use_cookie === 'true') {
                  if ($busy_cookie_exists === true || $busy_cookie_exists === '1') { // cookie exists, check version
                    $cookie_data = $_COOKIE["Drupal_visitor_".$cookie_name];
                    $jsondumpBusy = json_decode($cookie_data);
                    watchdog('occuspace_data', 'Occuspace busy data used from cookie. ID - %id', ['%id'=>$row->idNumber]);
                  } else { // cookie does not exists, need to create it and save it.
                    $make_busy_api_call = true;
                    $save_new_busy_cookie = true;
                  }
                }

                if ($make_busy_api_call === true) { // based on settings, api call for spaces needs to be ran
                  $busyOpts = [
                    'http' => [
                      'method' => 'GET',
                      'header' => 'Authorization: Bearer ' . $occuspaceAuthID,
                    ],
                  ];
                  $contextBusy = stream_context_create($busyOpts);
                  $newfileBusy = file_get_contents($busyUrl . '/' . $row->idNumber . '/now', FALSE, $contextBusy);
                  watchdog('occuspace_data', 'Occuspace busy data fetched from api. Url - %url', ['%url' => $busyUrl . '/' . $row->idNumber . '/now']);
                  $jsondumpBusy = json_decode($newfileBusy);
                }

                if ($save_new_busy_cookie === true) { // based on settings, cookie needs saved for this data to eliminate api call until version changes in settings
                  $newDataBusy = json_encode($jsondumpBusy);
                  $expireTime = intval($occuspace_busy_data_cookie_time)*60;
                  setrawcookie('Drupal.visitor.' . $cookie_name, rawurlencode($newDataBusy), REQUEST_TIME + $expireTime, '/');
                }

                $row->count = isset($jsondumpBusy->data->count) ? $jsondumpBusy->data->count : '';
                $row->percentage = isset($jsondumpBusy->data->percentage) ? round($jsondumpBusy->data->percentage * 100, 2) : '';
                $row->timestamp = isset($jsondumpBusy->data->timestamp) ? $jsondumpBusy->data->timestamp : '';
                $row->chartChoice = $chartChoice;
                $view->result[] = $row;
            }
        }

        //options for the query settings inside the view GUI
        function option_definition() {
            $options = parent::option_definition();
            $options['occuspace_chart_choice'] = ['default' => 'noGauge'];
            $options['occuspace_data_format_type'] = ['default' => 'people'];
            $options['occuspace_pieHole'] = ['default' => 65];
            $options['occuspace_slice_text_color'] = ['default' => 'white'];
            $options['occuspace_available_label'] = ['default' => 'Available'];
            $options['occuspace_count_label'] = ['default' => 'People'];
            $options['occuspace_green_color'] = ['default' => '#00B239'];
            $options['occuspace_yellow_color'] = ['default' => '#FFC300'];
            $options['occuspace_red_color'] = ['default' => '#B20000'];
            $options['occuspace_font_size'] = ['default' => 17];
            $options['occuspace_color_change'] = ['default' => 'true'];
            return $options;
        }

        //Creating the fields for the query settings in the view GUI
        function options_form(&$form, &$form_state) {
            parent::options_form($form, $form_state);
            $form['occuspace_data_format_type'] = [
                '#title' => t('Data Format'),
                '#type' => 'radios',
                '#description' => 'How would you like the data formatted?',
                '#default_value' => $this->options['occuspace_data_format_type'],
                '#options' => [
                    'people' => t('Number of people'),
                    'percentage' => t('Percentage of Capacity'),
                ],
            ];
            $form['occuspace_available_label'] = [
                '#title' => t('Available label'),
                '#type' => 'textfield',
                '#description' => 'The label for the chart indicating the number of people that can still enter the zone.',
                '#default_value' => $this->options['occuspace_available_label'],
            ];
            $form['occuspace_count_label'] = [
                '#title' => t('Count label'),
                '#type' => 'textfield',
                '#description' => 'The label for the chart indicating the number of people that are in the zone.',
                '#default_value' => $this->options['occuspace_count_label'],
            ];
            $form['occuspace_font_size'] = [
                '#title' => t('Font Size'),
                '#type' => 'textfield',
                '#description' => 'Enter the font size (px) of the chart text.',
                '#default_value' => $this->options['occuspace_font_size'],
            ];
            $form['occuspace_chart_choice'] = [
                '#title' => t('Pie - Chart style'),
                '#type' => 'radios',
                '#description' => 'Choose between a speedometer type gauge pie chart or a half donut type pie chart. *Pie chart only',
                '#default_value' => $this->options['occuspace_chart_choice'],
                '#options' => [
                    'yesGauge' => t('Speedomoter/Gauge'),
                    'noGauge' => t('Half Donut'),
                ],
            ];
            $form['occuspace_pieHole'] = [
                '#title' => t('Pie - Pie Hole'),
                '#type' => 'textfield',
                '#description' => 'The blank space in the middle of the pie chart. Enter a number between 0 and 1. To shut your piehole, enter 0. *Pie chart only',
                '#default_value' => $this->options['occuspace_pieHole'],
                '#size' => 3,
            ];
            $form['occuspace_slice_text_color'] = [
                '#title' => t('Pie - Slice Text Color'),
                '#type' => 'textfield',
                '#description' => 'The color (hex or color name) for the text on the pie slices. *Pie chart only',
                '#default_value' => $this->options['occuspace_slice_text_color'],
            ];
            $form['occuspace_color_change'] = [
                '#title' => t('Pie donut and Bar - Color Changing based on percentage?'),
                '#type' => 'radios',
                '#description' => 'Do you want the slice or bar to be green/yellow/red based on percentage of capacity? *Bar and pie donut type only',
                '#default_value' => $this->options['occuspace_color_change'],
                '#options' => [
                    'true' => t('Yes, auto-change color)'),
                    'false' => t('No, set to custom color in field settings)'),
                ],
            ];
            $form['occuspace_green_color'] = [
                '#title' => t('First section color'),
                '#type' => 'textfield',
                '#description' => 'The color for the first section of the chart (hex or color name). Default is green.',
                '#default_value' => $this->options['occuspace_green_color'],
            ];
            $form['occuspace_yellow_color'] = [
                '#title' => t('Second section color'),
                '#type' => 'textfield',
                '#description' => 'The color for the second section of the chart (hex or color name). Default is yellow.',
                '#default_value' => $this->options['occuspace_yellow_color'],
            ];
            $form['occuspace_red_color'] = [
                '#title' => t('Last section color'),
                '#type' => 'textfield',
                '#description' => 'The color for the last section of the chart (hex or color name). Default is red.',
                '#default_value' => $this->options['occuspace_red_color'],
            ];
        }
    }