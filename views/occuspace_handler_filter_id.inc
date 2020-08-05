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
    }
}