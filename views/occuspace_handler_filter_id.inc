<?php
/**
 * Created by PhpStorm.
 * User: smit0015
 * Date: 7/28/20
 * Time: 3:57 PM
 */

class occuspace_handler_filter_id extends views_handler_filter_numeric {
    public function query() {
        // $this->value is not empty when the above form has been filled out.
        if (empty($this->value)) {
            return;
        }

        $this->ensure_my_table();
        // Field will contain the value of the 'real field' setting defined at hook_views_data_alter().
//        $this->real_field = 'idNumber';
//        $field = "$this->table_alias.$this->real_field";
//dsm($this->options);
//        dsm($this->query);
        // Adds a condition to filter for content created on a given month.
//        $this->query->add_where_expression($this->options['group'], 'MONTH(FROM_UNIXTIME(' . $field . ')) = ' . $this->value[0]);
//        $this->query
//            ->add_where($this->options['group']
//            ->condition('idNumber', '169', 'like'));
//        dsm('what');
//        dsm($this->query);
    }
}