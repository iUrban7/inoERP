<?php

 $continue = true;
 $readonly1 = false;
 $read_access = false;
 $write_access = false;
 $update_access = false;
 $access_level = null;
 if (empty($mode)) {
  if (!empty($_GET['mode'])) {
   $mode = $_GET['mode'];
  } else {
   $path_access = -99;
   $mode = 1;
  }
 }
 if ($mode > 8) {
  $readonly = false;
 } else {
  $readonly = true;
 }
 if ($continue) {
  include_once (dirname(__FILE__) . '/../basics/basics.inc');
  global $f;
  global $ie;
  global $s;
  global $dbc;
  global $si;
  $file = null;
  /*
   * Check if class is content. If yes, check if any url alias exists
   */
  if (!empty($class_names) && is_array($class_names) && (in_array('content', $class_names))) {
   $alias = $_SERVER['QUERY_STRING'];
   $orginal_path_i = ext_url_alias::find_by_urlAlias($alias);
   if (!empty($orginal_path_i)) {
    $orginal_path = $orginal_path_i->original_url;
    $path_links = explode('&', $orginal_path);
    foreach ($path_links as $path_link) {
     $path_link_i = explode('=', $path_link);
     $_GET[$path_link_i[0]] = $path_link_i[1];
    }
   }
  }
  /* --------------------------------------------------------------------------------------------------------------
   * First call the theme class to find out the selected theme.
   * Select the default theme if no theme found
   */
  $selected_theme = 'default';
  defined('THEME_DIR') ? null : define('THEME_DIR', HOME_DIR . DS . 'themes' . DS . $selected_theme);
  defined('THEME_URL') ? null : define("THEME_URL", HOME_URL . 'themes/' . $selected_theme);
// if(!empty($si->maintenance_cb) && ($si->maintenance_cb == 1) && ($_SESSION['user_roles'][0] !='ADMIN')){
//	site_maintenance($si->maintenance_msg);
// }
 } else {
  return;
 }
 /* Check session */
 if (empty($_SESSION['user_id'])) {
  $_SESSION['orginal_page'] = $_SERVER["HTTP_HOST"] . $_SERVER["REQUEST_URI"];
 }
 /* -----------------------------------------------------------------------------------------------------
   Check if delete request. If yes, complete and return
  */
 if ((!empty($_GET['delete'])) && ($_GET['delete'] == 1)) {
  $data_array = $_GET;
  ino_delete($data_array);
  $continue = false;
  return;
 }

 /* ---------------------------------------------------------------------------------------------------------------
   check if its a search or select page
  * First search for select if not found then check for search page
  * use $noContextMenu or $noBlock
  */
 if ((isset($hideContextMenu)) && ($hideContextMenu)) {
  $showContextMenu = false;
 } else {
  $showContextMenu = true;
 }

 if ((isset($hideBlock)) && ($hideBlock)) {
  $showBlock = false;
 } else {
  $showBlock = true;
 }



 /* --------------------------------------------------------------------------------------------------------------
  * First call the role path to verify if the role/user has access to the page.
  * If the page need login, redirect users to login page
  */
 if (!empty($_POST['className']) && $_POST['className'] == 'comment') {
  $dont_check_login = true;
 }
 if (!empty($dont_check_login)) {
  //do nothing
 } else {
  $session->redirect_login();
 }

 /* --------------------------------------------------------------------------------------------------------------
  * Get all the varibles fro $_GET 
  * 
  */
 $current_url = "http://$_SERVER[HTTP_HOST]$_SERVER[REQUEST_URI]";
 $pageno = !(empty($_GET['pageno'])) ? (int) $_GET['pageno'] : 1;
 if (!empty($_GET['per_page']) && ($_GET['per_page'] == "all" || $_GET['per_page'][0] == "all")) {
  $per_page = 50000;
 } else if (!empty($_GET['per_page'])) {
  $per_page = (is_array($_GET['per_page'])) ? $_GET['per_page'][0] : $_GET['per_page'];
 }
 $per_page = empty($per_page) ? 10 : $per_page;
 $search_order_by = !(empty($_GET['search_order_by'])) ? $_GET['search_order_by'] : '';
 $search_asc_desc = !(empty($_GET['search_asc_desc'])) ? $_GET['search_asc_desc'] : '';
 $query_string = $_SERVER['QUERY_STRING'];

//page number details-------------------------
 if (!empty($query_string)) {
  if (strpos($query_string, 'pageno') !== false) {
   $pageno_value = $_GET['pageno'];
   $pageno_string = 'pageno=' . $pageno_value;
   $new_query_string1 = str_replace($pageno_string, "", $query_string);
   $query_string = $new_query_string1;
  }
 }
//---------------------------------------------
//remove double & from query string
 if (!empty($query_string)) {
  if (strpos($query_string, '&&') !== false) {
   $query_string = str_replace('&&', '&', $query_string);
  }
 }
 /*
   ---------------------------------------------------------------------------------------------------------------
  *  Determine the class names in case of POST data
  *  
  */
 if (!empty($_POST['className'])) {
  unset($class_names);
  $class_names[] = $_POST['className'];
 }

//if class names comes as a single value then convert it to an array and all dependent class names
 if ((!empty($class_names)) && (!(is_array($class_names)))) {
  $class_names_str = $class_names;
  unset($class_names);
  $class_names = [];
  $class_names[] = $class_names_str;
  if (!(strpos($_SERVER['SCRIPT_NAME'], '/search.php'))) {
   if (property_exists($class_names_str, 'dependent_classes')) {
    foreach ($class_names_str::$dependent_classes as $key => $dep_class_name) {
     array_push($class_names, $dep_class_name);
    }
   }
  }
 }


//add comment class for all if not POST
 if (empty($_POST)) {
  array_push($class_names, 'comment');
 }

 /* --------------------------------------------------------------------------------------------------------------
  * Initalize all the classes in the module
  */
 if (($continue) && (!empty($class_names))) {
  foreach ($class_names as $class) {
   $module = property_exists($class, 'module') ? $class::$module : 'module';
   $primary_column = property_exists($class, 'primary_column') ? $class::$primary_column : false;
   $primary_column2 = property_exists($class, 'primary_column2') ? $class::$primary_column2 : false;
   $primary_column3 = property_exists($class, 'primary_column3') ? $class::$primary_column3 : false;
   $key_field = property_exists($class, 'key_column') ? $class::$key_column : $class . '_id';
   $table_name = property_exists($class, 'table_name') ? $class::$table_name : $class;
//	$table_name = $class;
   $$class = new $class;

////////////////////////////////////////////////////////////////////
   if (isset($mode) && ($mode > 9)) {
    $data = [ 'Loader Class Defination @ line 102!' => $$class];
    array_push($ie->page_exception, $data);
   }
   /////////////////////////////////////////////////////////////////

   if (property_exists($class, 'field_a')) {
    if ((empty($$class->field_a))) {
     $$class->field_a = get_dbColumns($class);
    }
    foreach ($$class->field_a as $key => $value) {
     $$class->$value = NULL;
    }
   }

   //if not posted
   if (empty($_POST)) {
    $class_id = property_exists($class, 'primary_column') ? $class::$primary_column : $class_id = $class . '_id';
    if (!empty($_GET[$class_id])) {
     $class_id_value = $_GET[$class_id];
    } else {
//     pa($_SESSION);
     if (property_exists($$class, 'profile_default')) {
      if (!empty($$class->profile_default) && (is_array($$class->profile_default))) {
       foreach ($$class->profile_default as $field_name => $profile_name) {
//        pa($prl->find_default_profile($profile_name));
        $$class->$field_name = $prl->find_default_profile($profile_name)->level_value;
       }
      }
     }
    }

    //-------------------------------------------------------------------------------------
    if (!empty($class_id_value) && (!is_array($class_id_value))) {
     $$class->findBy_id($class_id_value);
     foreach ($$class as $key => $value) {
      if ((!empty($value)) && (property_exists($$class, $key))) {
       $$class->$key = $value;
      }
     }
     $file = file::find_by_reference_table_and_id($table_name, $class_id_value);
     $comments = comment::comment_list($table_name, $class_id_value, $per_page, $pageno, $query_string, $class_id_value);
     if ($class != 'category') {
      $category = category::find_by_reference_table_and_id($table_name, $class_id_value);
     }
     if ((!empty($$class->$primary_column)) && (!empty($class::$addressField_array))) {
      foreach ($class::$addressField_array as $key => $value) {
       if (!empty($$class->$value)) {
        $address = address::find_by_id($$class->$value);
       } else {
        $address = new address();
       }
       $$value = address::show_adrees_details($address);
      }
     }
     if (method_exists($$class, '_before_showing')) {
      $$class->_before_showing();
     }
    }

    unset($class_id_value);
    /*
     * ---------------------------------------------------SEARCH-----------------------------------------------
     */

    if ((!empty($_GET['submit_search'])) && (!empty($_GET['search_class_name'])) && ($_GET['search_class_name'] === $class)) {
//	 if ((!empty($_GET['submit_search'])) && (!empty($_GET['search_class_name']) && $class !='comment' )) {
     $search_array = $$class->field_a;
     global $search_result_statement;

     //pre populate
     if (method_exists($$class, 'search_pre_populate')) {
      $ppl = call_user_func(array($$class, 'search_pre_populate'));
      if (!empty($ppl)) {
       foreach ($ppl as $search_key => $search_val) {
        $_GET[$search_key] = $search_val;
       }
      }
     }

     if (empty($_GET['column_array'])) {
      if (property_exists($class, 'column')) {
       $column_array = $$class->column;
      } else {
       $column_array = $$class->field_a;
      }
     } else {
      $column_array = unserialize(base64_decode($_GET['column_array']));
     }
     if (!empty($_GET['new_column'])) {
      $new_column = $_GET['new_column'];
      if (!empty($new_column)) {
       foreach ($new_column as $key => $value) {
        if ((!(is_array($value))) && (!empty($value))) {
         array_push($column_array, $value);
        }
       }
      }
     }

     $whereFields = [];
     $sqlWhereFields = [];
     $existing_search = [];
//to check number of criterias
     $noof_criteria = 0;
     foreach ($search_array as $key => $value) {

      if (!empty($_GET[$value][0])) {
       $entered_search_criteria = str_replace(' ', '%', trim($_GET[$value][0]));
       array_push($existing_search, $value);
       if (strpos($value, '_ID') !== false) {
        $whereFields[] = sprintf("%s = %s ", $value, trim($entered_search_criteria));
       } else if (substr($entered_search_criteria, 0, 1) == '=') {
        $whereFields[] = sprintf("%s = '%s' ", $value, trim(substr($entered_search_criteria, 1)));
       } else if (substr($entered_search_criteria, 0, 2) == '!=') {
        $whereFields[] = sprintf("%s != '%s' ", $value, trim(substr($entered_search_criteria, 2)));
       } else if (substr($entered_search_criteria, 0, 1) == '>') {
        $whereFields[] = sprintf("%s > '%s' ", $value, trim(substr($entered_search_criteria, 1)));
       } else if (substr($entered_search_criteria, 0, 1) == '<') {
        $whereFields[] = sprintf("%s < '%s' ", $value, trim(substr($entered_search_criteria, 1)));
       } else {
        $whereFields[] = sprintf("%s LIKE '%%%s%%'", $value, trim(mysql_prep($entered_search_criteria)));
       }
       $noof_criteria++;
      }

      if (!empty($_GET[$value][1])) {
       array_push($existing_search, $value);
       if (substr($_GET[$value][1], 0, 1) == '>') {
        $whereFields[] = sprintf("%s > '%s' ", $value, trim(substr($_GET[$value][1], 1)));
       } else if (substr($_GET[$value][1], 0, 1) == '<') {
        $whereFields[] = sprintf("%s < '%s' ", $value, trim(substr($_GET[$value][1], 1)));
       }
       $noof_criteria++;
      }
     }
     if ((!empty($_GET['function_name'])) && (method_exists($$class, $_GET['function_name']))) {
      $function_name = $_GET['function_name'];
      $search_counts = $function_name . '_search_counts';
      $search_records = $function_name . '_search_records';
      $whereClause = implode(" AND ", $whereFields);
      $_GET['whereClause'] = $whereClause;
      $total_count = call_user_func(array($$class, $search_counts), $_GET);
      $search_result = call_user_func(array($$class, $search_records), $_GET);
      if (!empty($per_page) && !empty($total_count)) {
       $pagination = new pagination($pageno, $per_page, $total_count);
       $pagination->setProperty('_query_string', $query_string);
       $pagination_statement = $pagination->show_pagination();
      }
     } else if (method_exists($$class, 'search_records')) {
      $whereClause = implode(" AND ", $whereFields);
      $_GET['whereClause'] = $whereClause;
      $total_count = call_user_func(array($$class, 'search_counts'), $_GET);
      $search_result = call_user_func(array($$class, 'search_records'), $_GET);
      if (!empty($per_page) && !empty($total_count)) {
       $pagination = new pagination($pageno, $per_page, $total_count);
       $pagination->setProperty('_query_string', $query_string);
       $pagination_statement = $pagination->show_pagination();
      }
     } else {
      require_once 'loader_search.inc';
     }

     if (method_exists($class, 'search_add_extra_fields')) {
      $class::search_add_extra_fields($search_result);
     }

     if (property_exists($class, 'search')) {
      foreach ($$class->search as $searchParaKey => $searchParaValue) {
       $s->setProperty($searchParaKey, $searchParaValue);
      }
     }

     $s->setProperty('result', $search_result);
     $s->setProperty('_searching_class', $class);
     $s->setProperty('_per_page', $per_page);
     $s->setProperty('primary_column_s', $primary_column);
     $s->setProperty('column_array_s', $column_array);
     $search_result_statement = $search->search_result_op();
    }
   }
   //---------------------------------------END OF SEARCH------------------------------------------------

   /* -------------------------------------START OF SAVE---------------------------------------------------- */ elseif ((empty($_POST['download'])) && !empty($_POST['className'])) {
    $inosave = new inosave();
    $inosave->setProperty('_module', $module);
    $inosave->setProperty('_classname', $class);
    if (!empty($_POST['detail_classname'])) {
     $inosave->setProperty('_detail_classname', $_POST['detail_classname']);
    }
    $inosave->setProperty('_key_field', $key_field);
    $inosave->setProperty('_primary_column', $primary_column);
    $inosave->setProperty('_primary_column2', $primary_column2);
    $inosave->setProperty('_primary_column3', $primary_column3);
    $inosave->json_save();
//    pa($inosave);
    $dbc->confirm();
    $continue = false;
    return;
   }
   /* -------------------------------------END OF SAVE---------------------------------------------------- */
  }
  /* -------------------------------------START OF SAVE---------------------------------------------------- */
 } else {
  $continue = false;
  return;
 }

//Assign dependet classes to class_second & class_third and so on
 $class_first = $class = $class_names[0];
 $$class_first = $$class_names[0];
 $class_id_first = property_exists($class, 'primary_column') ? $class::$primary_column : $class_id = $class . '_id';

 if (isset($class_names[1])) {
  $class_second = $class_names[1];
  $$class_second = $$class_names[1];
  $line_obj = $class_second . '_object';
  if (property_exists($class, 'primary_column')) {
   $primar_column_class1 = $class::$primary_column;
   $order_by_field = !empty($class_second::$query_order_by_field) ? $class_second::$query_order_by_field : '';
   $order_by_value = !empty($class_second::$query_order_by_value) ? $class_second::$query_order_by_value : '';
   $$line_obj = [];
   array_push($$line_obj, $$class_second);
   if (!empty($$class->$primar_column_class1) && (!empty($class_second::$parent_primary_column))) {
    $$line_obj = $$class_second->findBy_parentId($$class->$primar_column_class1, $order_by_field, $order_by_value);
    if (empty($$line_obj)) {
     $$line_obj = [];
     array_push($$line_obj, $$class_second);
    }
   }
  }
 }

 if (isset($class_names[2])) {
  $class_third = $class_names[2];
  $$class_third = $$class_names[2];
 }

 if (isset($class_names[3])) {
  $class_fourth = $class_names[3];
  $$class_fourth = $$class_names[3];
 }
///////////////////////////set the mode & readonly - readonly1
 if (property_exists($class, 'primary_column')) {
  $primary_column = $class::$primary_column;
 }
 if (!empty($$class->$primary_column)) {
  $readonly1 = true;
 } else {
  $readonly1 = false;
 }

 if ($readonly) {
  $f->readonly = $readonly;
 }


 if (($class == 'content') && (!empty($content->subject))) {
  $pageTitle = $content->subject;
 } else {
  $pageTitle = property_exists($$class, 'pageTitle') ? $$class->pageTitle : 'inoERP';
 }

/////////////////////////////////////////////////////
 if (isset($mode) && ($mode > 9)) {
  $data = [ 'POST as in Loader file @ line 229!' => $_POST];
  array_push($ie->page_exception, $data);
 }
/////////////////////////////////////////////////////

 if (!empty($_GET['delete'])) {
  $continue = false;
  return;
 }


 /* -----------------------------------------include all include files -------------------------------------- */
 if ($continue) {
  $reflector = new ReflectionClass($class);
  $fnname = $reflector->getFileName();
  $fdir = dirname($fnname);

  $include_file_names = glob($fdir . DS . "*.inc");
  $css_file_names = glob($fdir . "/*.css");
  $template_file_names = glob($fdir . DS . "*_template.php");

  if ($showContextMenu) {
   $js_file_names = glob($fdir . "/*.js");
   $js_file_paths = [];
   foreach ($js_file_names as $key => $js_file_name) {
    if (strpos($js_file_name, 'modules')) {
     $js_file_path = str_replace('\\', '/', substr($js_file_name, strpos($js_file_name, 'modules')));
    } else if (strpos($js_file_name, 'extensions')) {
     $js_file_path = str_replace('\\', '/', substr($js_file_name, strpos($js_file_name, 'extensions')));
    }
    array_push($js_file_paths, $js_file_path);
   }
  }

  $css_file_paths = [];
  foreach ($css_file_names as $key => $css_file_name) {
   if (strpos($css_file_name, 'modules')) {
    $css_file_path = str_replace('\\', '/', substr($css_file_name, strpos($css_file_name, 'modules')));
   } else if (strpos($css_file_name, 'extensions')) {
    $css_file_path = str_replace('\\', '/', substr($css_file_name, strpos($css_file_name, 'extensions')));
   }
   else if (strpos($css_file_name, 'engine')) {
    $css_file_path = str_replace('\\', '/', substr($css_file_name, strpos($css_file_name, 'engine')));
   }
   array_push($css_file_paths, $css_file_path);
  }

  foreach ($include_file_names as $key => $pathname) {
   include_once "$pathname";
  }
 } else {
  $continue = false;
  echo "<h2>Could n't call the header</h2>";
  return;
 }
//change mode for contents

 if (!empty($_SESSION['user_roles'])) {
  if (in_array('ADMIN', $_SESSION['user_roles'])) {
   $access_level = 9;
   $read_access = $write_access = $update_access = true;
  } else {
   if (!empty($_SESSION['role_access'])) {
//    pa($_SESSION['role_access']);
    foreach ($_SESSION['role_access'] as $obj_name => $acc_level) {
     if ($obj_name == $class) {
      $access_level = $acc_level;
      break;
     }
    }
//    echo "<br> class is $class access_level is $access_level ";
   }
  }

  if ((!empty($access_level)) && ($access_level != 9)) {
   require_once 'loader_access.inc';
  } else if (!empty($access_level) && $access_level == 9) {
   $read_access = $write_access = $update_access = true;
  }
 } else {
  $mode = 2;
 }

 /* --------------------------------------------------------------------------------------------------------------
  * Include Cache
  * call the header file
  * 
  */
 if ($continue) {
  include_once(THEME_DIR . '/header.inc');
 } else {
  $continue = false;
  echo "<h2>Could n't call the header</h2>";
  return;
 }

 /* --------------------------------------------------------------------------------------------------------------
  * Only for test
  * Exit loader */
//$continue = false;
//if ($continue) {
// echo "<h2>This wont be printed</h2>";
//} else {
// echo "<h2>Exiting loader</h2>";
// return;
//}
?>