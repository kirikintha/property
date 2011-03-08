<?php
/**
 * Property Module Admin
 */

function _ram_property_admin() { //Main Admin settings
  
  $form = array();
  
  $requirements = ramAdmin::_requirements();

  /**
   * @param string variable_get('ram_property_setup', NULL) a string that shows the phase you have completed in the setup
   * init = the initial setup instructions, set up the API first
   * test_connection = add in your website's ip address and domain and test the connection
   * tokens = set the tokens for this import
   * content = create the content type associations, you need to have a content type here
   * complete = setup has completed, now you can tune the server and set an import threshold
   */
  switch (variable_get('ram_property_setup', NULL)) { //form build with different stages
    
    case 'init':
    
      $header = _ram_property_setup_header($requirements); //header objects, requirements
      $init   = _ram_property_setup_init();
      $form   = array_merge($form, $init);
    
    break;
    
    case 'complete';
      
      if (variable_get('property_firstrun_imported', NULL) == TRUE) { //If we have a first run, then show cron options
      
        $admin = _ram_property_setup_complete();
        $form  = array_merge($form, $admin);
        
      }
      
      $firstrun = _ram_property_setup_firstrun();
      $form     = array_merge($form, $firstrun);

      if (variable_get('property_can_curl_images', TRUE) == TRUE) {
        $importImages = _ram_property_setup_import_images();
        $form     = array_merge($form, $importImages);
      }
      
      $header = _ram_property_setup_header($requirements, 'runtime'); //header objects, requirements

    case 'content':
      
      $content = _ram_property_setup_content();
      $form    = array_merge($form, $content);
      
    case 'cache':
      //If we are not in the property content type stage, then show this. We want to stop re-caching when assigning a proprty content type, but want to keep it in the desired setup stage order.
      if (variable_get('ram_property_setup', NULL) != 'content') {
        $cache = _ram_property_setup_cached();
        $form  = array_merge($form, $cache);
      }
      
    case 'filters':
      
      $filters = _ram_property_setup_filters();
      $form    = array_merge($form, $filters);
      
    case 'tokens':
      
      $tokens = _ram_property_setup_tokens();
      $form   = array_merge($form, $tokens);
      
    case 'test_connection':
  
      $test = _ram_property_setup_test_connection();
      $form = array_merge($form, $test);
      
      $header = _ram_property_setup_header($requirements); //header objects, requirements
      
    break;
    
  }
  
  switch (variable_get('ram_property_setup', NULL)) { //switch between header objects
    
    case 'init':
    
      $header = _ram_property_setup_header($requirements, 'connection'); //header objects, requirements
      $form   = array_merge($header, $form);
    
    break;
    
    case 'complete':
      
      $header = _ram_property_setup_header($requirements, 'connection'); //header objects, requirements
      $form   = array_merge($header, $form);
      
      $header = _ram_property_setup_header($requirements, 'runtime'); //header objects, requirements
      $form   = array_merge($header, $form);
    
    break;

    case 'content':
    case 'cache':  
    case 'filters':
    case 'tokens':
    case 'test_connection':
      
      $header = _ram_property_setup_header($requirements, 'connection'); //header objects, requirements
      $form   = array_merge($header, $form);
      
      $header = _ram_property_setup_header($requirements, 'setup'); //header objects, requirements
      $form   = array_merge($header, $form);
      
    break;
    
  }

  return system_settings_form($form);
  
}

/**
 * Validate Admin Form
 * @todo - since I have added more form fields, I need to make sure all the variables clear properly.
 */
function _ram_property_admin_validate(&$form_state, $form_values) {
  
  //reset - leave this at the end, since we probably reset a lot of stuff here
  if ((strstr(strtolower($form_values['values']['op']), 'reset'))) { //else, if we have reset or anything else, then kill the variables
    
    drupal_set_message(t('You have reset the RAM Module to the default settings, please start over again. If you have reset this dialogue after making an import, you may have to start everything over again.'), 'warning');
    
    variable_del('property_batch_chunk_maximum_limit');
    variable_set('ram_property_connection_filtered', FALSE);
    variable_del('ram_property_query_maximum_limit_default');
    variable_del('property_content_type');
    variable_del('ram_property_connection_string'); //delete the connection string
    variable_del('ram_property_website');
    variable_del('ram_property_ipaddress');
    variable_del('ram_property_orderby');
    variable_set('ram_property_api_host', NULL);
    variable_set('ram_property_api_host_filename', NULL);
    variable_set('ram_property_connect_log', NULL);
    variable_set('ram_property_connection_tested', FALSE);

    variable_set('property_connection_tokenized', FALSE);
    variable_set('ram_property_connection_cached', FALSE);
    variable_set('property_firstrun_imported', FALSE); //This is our last setup step!
    variable_set('ram_property_areas', NULL);
    variable_set('ram_property_connection_filtered', FALSE);
    variable_set('ram_property_setup', 'init'); //reset to init
    
    //clear system cache;
    propertyCache::_clearCache('property_cache');
    
    //reset the token keys - this had to stay here, because for some reason Drupal screws up the array association - so odd...
    $ramKeys = array(
      'Mls_Num',
      'Prop_Class',
      'Prop_Type',
      'Area',
      'Asking_Price',
      'Address',
      'City',
      'State',
      'Zip',
      'Status',
      'Sale_Rent',
      'Num_Bedrms',
      'Fl_Baths',
      'Hlf_Bath',
      'Garg_Cap',
      'Garg_Typ',
      'Levels',
      'Basement',
      'Condition',
      'Reservations',
      'Number_Acres',
      'Price_Per_Acre',
      'Number_Units',
      'Agent_ID',
      'Agent_Name',
      'Agent_Phone',
      'Office_ID',
      'Office',
      'Office_Phone',
      'Subdivision',
      'School',
      'County',
      'Zone_District',
      'Legal',
      'Yr_Built',
      'Remodeled',
      'Ttl_Sqft',
      'Bldg_2_Ttl_Sqft',
      'Bldg_3_Ttl_Sqft',
      'Finished',
      'Unfinished',
      'Remarks',
      'Garage_Size',
      'Garage_Comments',
      'Extra_Descript',
      'Siz_Cmnt',
      'Taxes1',
      'Taxes1yr',
      'Taxes2',
      'Taxes2yr',
      'TaxesPer',
      'Current_Use',
      'Water_Rights',
      'Const_type',
      'Amenities',
      'virtual_tour',
      'Date_inserted',
      'Date_Modified',
      'LotSizeRange',
      'LotSizeActual',
      'Agent_ID_2',
      'Agent_Name_2',
      'Agent_Phone_2',
      'Office_ID_2',
      'Office_2',
      'Office_Phone_2',
   );
    
    foreach ($ramKeys as $key => $value) {
      
      $key    = ((int) $key) + 1; //the darn key is always one more...
      $keys[] = "key$key|$value";
      
    }
    
    variable_set('property_default_tokens', $keys); //Remember default tokens are actually part of the property module, but your module has control
    
    $keys = implode("\n", $keys);
    
    variable_set('property_tokens', $keys); //set up the main array for the admin area, this controls how the xml is imported and how the content type is made
    
  }

  if (strstr(strtolower($form_values['values']['op']), 'save')) { //validate on save
    
    if (variable_get('property_content_type', NULL) != NULL && variable_get('ram_property_connection_tested', FALSE) == TRUE && variable_get('property_connection_tokenized', NULL) == TRUE) {
      
      variable_set('ram_property_setup', 'complete');
      
    }

    switch (variable_get('ram_property_setup', NULL)) { //we need to change some things during the set up phase
      
      case 'complete';
      case 'content':
        
      case 'filters': //This is not the greatest validation
        
        (empty($form_values['values']['ram_property_orderby'])) ? form_set_error('ram_property_orderby', t('Please add a valid string to order by.')) : variable_set('ram_property_connection_filtered', TRUE);
        (variable_get('ram_property_connection_orderby', NULL) == TRUE && variable_get('ram_property_setup', NULL) == 'filters') ? variable_set('ram_property_setup', 'cache') : NULL ;
        (variable_get('ram_property_connection_filtered', NULL) == TRUE && variable_get('ram_property_setup', NULL) == 'filters') ? variable_set('ram_property_setup', 'cache') : NULL ;
         
      case 'tokens':
        
        (!empty($form_values['values']['property_tokens']) && variable_get('property_tokens', NULL) != $form_values['values']['property_tokens']) ? variable_set('property_tokens', $form_values['values']['property_tokens']) : NULL;
        
      case 'test_connection':
        
        variable_set('ram_property_website', $form_values['values']['ram_property_website']);
        variable_set('ram_property_ipaddress', $form_values['values']['ram_property_ipaddress']);
        
      case 'init':
      
        (empty($form_values['values']['ram_property_api_host']) || !propertyTrans::_validateURL($form_values['values']['ram_property_api_host'])) ? form_set_error('ram_property_api_host', t('You must set a hostname value, please add a valid host name.')): $host = $form_values['values']['ram_property_api_host'];
        (empty($form_values['values']['ram_property_api_host_filename'])) ? form_set_error('ram_property_api_host_filename', t('You must set an API file name value, please add a valid file name, with .php as the extension.')): $host_filename = $form_values['values']['ram_property_api_host_filename'];
        
        //print_r($host.$host_filename);
        $host = ($host && $host_filename) ? $host.$host_filename : NULL;
        if (!empty($host) && variable_get('ram_property_connection_string', NULL) != $host) { //if both file fields are present and accounted for, advance the setup, and force another diagnostic if they change the connection
          
          variable_set('ram_property_setup', 'test_connection'); //set the setup step we are in
          variable_set('ram_property_connection_string', $host); //set the connection string.
          
        }
          
      break;
      
    }
    
    if (!empty($form_values['values']['property_content_type']) && variable_get('property_content_type', NULL) != $form_values['values']['property_content_type']) {
      
      variable_set('property_content_type', $form_values['values']['property_content_type']);
      variable_set('ram_property_setup', 'content');
      
    }
    
  }
  
}

/**
 * Last Cache from RAM, for debugging the last call from RAM
 */
function _ram_property_last_cache() {

  $today = date('m_d_Y', strtotime('now'));
  $new   = propertyCache::_getCache('property_cache_new_cache');
  $xml   = propertyTrans::_validateData($new);
  
  if (is_object($xml)) { //If we have valid XML
    print t('!MSG', array('!MSG' => "<h2>Cache Data for: $today</h2>"));
    foreach ($xml as $item) { //go through each object, and show us the last cache
        
      print '<pre>';
        print_r($item);
      print '</pre>';
      
    }
    
  } else { //No New Cache
    
    print t('There is no new cached data at this time.');
    
  }
  
  exit();
  
}

/**
 * Last Cache from RAM, for debugging the last call from RAM
 */
function _ram_property_yesterdays_cache() {

  $yesterday       = date('m_d_Y', strtotime('now - 1 day'));
  $yesterdaysData  = propertyCache::_getCache('property_cache_cache_'.$yesterday);
  $yesterDaysCache = propertyTrans::_validateData($yesterdaysData);

  if (is_object($yesterDaysCache)) { //If we have valid XML

    print t('!MSG', array('!MSG' => "<h2>Cache Data for: $yesterday</h2>"));

    foreach ($yesterDaysCache as $item) { //go through each object, and show us the last cache

      print '<pre>';
        print_r($item);
      print '</pre>';

    }

  } else { //No New Cache

    print t('There is no new cached data for yesterday, at this time.');

  }

  exit();

}

/**
 * Current Quick Pull from RAM
 */
function _ram_property_quick_pull() {

  $manager = new ramManager;
    
  $query = array(
    'numresults' => variable_get('ram_property_query_maximum_limit_default', RAM_PROPERTY_QUERY_MAXIMUM_LIMIT_DEFAULT),
    'offset'     => 0,
    'orderby'    => variable_get('ram_property_orderby', 'Area ASC'),
  );
  
  $manager->qry = ramTrans::_query($query);
  
  $new = propertyTrans::_commit('runtime', $manager);

  //Get our Merged XML and validate it and if it is valid, rock it.
  $xml = propertyTrans::_validateData($new['data']);
  
  if (is_object($xml)) { //If we have valid XML
    
    foreach ($xml as $item) { //go through each object, and show us the last cache
        
      print '<pre>';
        print_r($item);
      print '</pre>';
      
    }
    
  } else { //No good XML
    
    print t('There is an XML error from RAM, could parse the XML feed.');
    
    print '<pre>';
      print_r($new);
    print '</pre>';
    
  }
  
  exit();
  
}


/**
 * Test Connection :: From _commit in js
 */
function _ram_property_test_connection($data = NULL) {
  
  $manager = new ramManager;
  $manager->qry = ramTrans::_query();
  
  $connect = propertyTrans::_commit('test_connection', $manager);
  
  $xml = propertyTrans::_validateData($connect['data']);
  $tested = (is_object($xml)) ? TRUE : FALSE;
  variable_set('ram_property_connection_tested', $tested); //set the tested variable
  
  if ($tested == TRUE) {

    //Add to our connection log.
    variable_set('ram_property_connect_log', $connect['log']);
    //Change our setup step if necessary.
    (variable_get('ram_property_setup', NULL) == 'test_connection' && variable_get('property_connection_tokenized', FALSE) === FALSE) ? variable_set('ram_property_setup', 'tokens') : variable_set('ram_property_setup', variable_get('ram_property_setup', 'tokens')) ;

    //Find the values returned by the query, and set a variable defining what the query limit is.
    variable_set('ram_property_query_items_total', (int) $xml->itemsTotal);

  }
  
  return $connect['data'];
  
}

/**
 * Generate Tokens :: From _commit in js
 * @todo - move this to admin.property
 */
function _ram_property_generate_tokens($data = NULL) {

  //Remove anything not a pipe or an alphanumeric value.
  $data = preg_replace("/[\r\n]+[\s\t]*[\r\n]+/","\n", $data);

  if (variable_get('ram_property_connection_tested', NULL) == TRUE && !empty($data)) {
    
    variable_set('property_tokens', $data);
    (variable_get('ram_property_setup', NULL) == 'tokens') ? variable_set('ram_property_setup', 'filters') : NULL;
    variable_set('property_connection_tokenized', TRUE);
    
  } else {
    
    variable_set('property_connection_tokenized', FALSE);
    
  }
  
  return $data;
  
}

/**
 * @name = _ram_property_retrieve_new_cache
 */
function _ram_property_retrieve_new_cache($data = NULL) {
  
  return ramCache::_retrieveNewCache();
  
}

/**
 * @name = _ram_property_analyze_new_cache
 */
function _ram_property_analyze_new_cache($data = NULL) {

  //If we are forcing an update, then make yesterdays cache into a _bak row, delete the original one and then re-instate yesterdays cache on _process_orphans.
  if ($data == 'FORCE_UPDATE') {
    //Set a message saying we are analyzing all properties.
    drupal_set_message('All Properties Have Been Updated.');
    //set update flag, so cron does not update the cache by accident.
    variable_set('ram_property_cron_run_lock', TRUE);
    //get params.
    $yesterday      = date('m_d_Y', strtotime('now - 1 day'));
    $yesterdaysData = propertyCache::_getCache('property_cache_cache_'.$yesterday);
    //Back-up yesterdays cache.
    propertyCache::_setCache('property_cache_cache_'.$yesterday.'_backup', $yesterdaysData, CACHE_PERMANENT, 'cache_property_suite');
    //Clear out yesterdays cache for analysis - re-instate the original at property orphans.
    propertyCache::_clearCache('property_cache_cache_'.$yesterday, 'cache_property_suite', FALSE);
  }
  
  return ramCache::_analyzeNewCache();
  
}

/**
 * @name = _ram_property_analyze_new_cache
 */
function _ram_property_analyze_new_images($data = NULL) {

  //If we are forcing an update, then make yesterdays cache into a _bak row, delete the original one and then re-instate yesterdays cache for images.
  if ($data == 'FORCE_UPDATE') {
    $yesterday       = date('m_d_Y', strtotime('now - 1 day'));
    $yesterdaysData  = propertyCache::_getCache('property_cache_cache_'.$yesterday.'_backup');
    //Re-in-state yesterdays cache.
    propertyCache::_setCache('property_cache_cache_'.$yesterday, $yesterdaysData, CACHE_PERMANENT, 'cache_property_suite');
    //Clear out yesterdays cache for analysis - re-instate the original at property orphans.
    propertyCache::_clearCache('property_cache_cache_'.$yesterday.'_backup', 'cache_property_suite', FALSE);
  }
  
  return ramCache::_analyzeNewCacheImages();
  
}

/**
 * @name = _ram_property_process_orphans
 */
function _ram_property_process_orphans( $data = null ) {

  return ramCache::_processOrphans();

}

/**
 * @name = _ram_property_import_first_run
 */
function _ram_property_import_first_run() {
  
  $manager = new ramManager;
  propertyTrans::_commit('batchProperties', $manager); //this commits to $manager->_batch
  
}

/**
 * @name = _ram_property_import_first_run
 */
function _ram_property_import_images() {

    $manager = new ramManager;
    propertyTrans::_commit('batchImages', $manager); //this commits to $manager->_batch
  
}

/**
 * @name = _ram_property_setup_header
 * @param array $requirements an array of requirements, defined by this module
 * @param string $stage a string that defines the stage this module is in
 *  stage values:
 *    connection
 *    setup
 *    runtime
 * @param integer $weight you can manipulate the form here
 */

function _ram_property_setup_header($requirements, $stage = 'setup', $weight = -50) {
  
  $name = 'status_report_' .$stage;
  $form[$name] = array(
    '#value'  => propertyAdmin::_themeStatusReport($requirements[$stage]),
    '#weight' => $weight,
    '#group'  => 'admin_main'
  );
  
  return $form;
  
}

/**
 * Setup Init
 */
function _ram_property_setup_init() {
  
  $form['admin_main'] = array(
    '#type' 	   => 'fieldset',
    '#title' 	   => t('RAM API Set-up'),
    '#collapsible' => TRUE,
    '#collapsed'   => FALSE,
    '#weight'	   => 50,
    '#group'       => 'admin_main'
  );
  
  $form['admin_main']['setup']['api']['ram_property_api_host'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Host'),
    '#description'    => t('Please enter in the host name for this RAM API. ex: http://vail.interactiveproperties.com/ <em>Please include the protocol for your connection, and trailing slash.</em>'),
    '#default_value'  => variable_get('ram_property_api_host', NULL),
    '#required'       => TRUE,
    '#weight'         => -49,
    '#group'          => 'admin_main'
  );
  
  $form['admin_main']['setup']['api']['ram_property_api_host_filename'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Host File Name'),
    '#description'    => t('Please enter in the file name of the API file for this host. ex: vailAPI.php <em>Do not include preceding or trailing slashes!</em>'),
    '#default_value'  => variable_get('ram_property_api_host_filename', NULL),
    '#required'       => TRUE,
    '#weight'         => -49,
    '#group'          => 'admin_main'
  );
  
  $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
  
  return $form;
  
}

/**
 * Setup Test Connection
 */
function _ram_property_setup_test_connection() {
  
  $form['auth_info'] = array(
		'#type' 				=> 'fieldset',
		'#title' 				=> t('Connection'),
		'#collapsible' 	=> TRUE,
		'#collapsed' 		=> FALSE,
		'#weight'				=> 50,
    '#group'        => 'auth_info'
	);
  
  $form['auth_info']['ram_property_website'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Website'),
    '#description'    => t('Please enter this website\'s url, if the system has not automatically entered it for you, or if it is different than the one reported here.'),
    '#prefix'         => '<p>'. variable_get('ram_property_connect_log', NULL) .'</p>',
    '#default_value'  => variable_get('ram_property_website', $_SERVER['HTTP_HOST']),
    '#required'       => TRUE,
    '#weight'         => -49,
    '#group'          => 'auth_info'
 );
  
  $form['auth_info']['ram_property_ipaddress'] = array(
    '#type'           => 'textfield',
    '#title'          => t('IP Address'),
    '#description'    => t('Please enter the ip address for this website. If you are on a virtual server you may have to check with your host for the proper ip address.'),
    '#default_value'  => variable_get('ram_property_ipaddress', $_SERVER['SERVER_ADDR']),
    '#required'       => TRUE,
    '#weight'         => -48,
    '#group'          => 'auth_info'
 );
  
  $form['auth_info']['test_connection'] = array(
		'#type'         => 'submit',
		'#value'        => t('Test Connection'),
    '#prefix'       => '<div id="test-progress"></div><p>',
    '#suffix'       => t('Test the connection right now, you need to do this once, to ensure that we are connecting to the server.') .'</p>',
    '#required'     => TRUE,
    '#weight'       => -47,
    '#group'        => 'auth_info'
 );
  
  $form['auth_info']['ram_property_api_host'] = array(
    '#type'           => 'hidden',
    '#default_value'  => variable_get('ram_property_api_host', NULL),
    '#weight'         => -46,
    '#group'          => 'auth_info'
 );
  
  $form['auth_info']['ram_property_api_host_filename'] = array(
    '#type'           => 'hidden',
    '#default_value'  => variable_get('ram_property_api_host_filename', NULL),
    '#weight'         => -45,
    '#group'          => 'auth_info'
 );
  
  $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
  
  return $form;
  
}

/**
 * Set up content Associations
 */
function _ram_property_setup_tokens() {
  
  return propertyAdmin::_setupTokenForm();
  
}

/**
 * Set up first run
 */
function _ram_property_setup_cached() {
  
  return propertyAdmin::_setupCacheForm($requirements);
  
}

/**
 * Set up filters
 */
function _ram_property_setup_filters() {
  
  $form['filters'] = array(
    '#type' 				=> 'fieldset',
    '#title' 				=> t('Filters'),
    '#collapsible' 	=> TRUE,
    '#collapsed' 		=> FALSE,
    '#weight'				=> 48,
    '#group'        => 'filters'
	);
  
  $form['filters']['ram_property_broker_key'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Broker Key'),
    '#description'    => t('This is the token you wish to use to identify the Broker or Agent ID to the system. ex: [Agent_ID]'),
    '#default_value'  => variable_get('ram_property_broker_key', '[Agent_ID]'),
    '#required'       => TRUE,
    '#weight'         => -50,
    '#group'          => 'filters'
 );
  
  $form['filters']['ram_property_image_key'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Image Key'),
    '#description'    => t('The import key to use for the image key. Usually this key is: imagesavailable, but you may have another key to use.'),
    '#default_value'  => variable_get('ram_property_image_key', 'imagesavailable'),
    '#required'       => TRUE,
    '#weight'         => -49,
    '#group'          => 'filters'
 );
  
  $form['filters']['ram_property_image_path'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Image Path'),
    '#description'    => t('Please set the image path to import images from. ex: http://www.vail.interactiveproperties.com/images/listings/'),
    '#default_value'  => variable_get('ram_property_image_path', NULL),
    '#required'       => TRUE,
    '#weight'         => -48,
    '#group'          => 'filters'
 );
  
  $form['filters']['ram_property_property_key'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Property Key'),
    '#description'    => t('The property key is the token that identifies the unique ID for each property. This is usually the property ID or the MLS number, if unsure use the MLS number.'),
    '#default_value'  => variable_get('ram_property_property_key', '[Mls_Num]'),
    '#required'       => TRUE,
    '#weight'         => -47,
    '#group'          => 'filters'
 );
  
  $form['filters']['ram_property_orderby'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Order By'),
    '#description'    => t('Set a valid string to order this query by. Use the token list you created to chose, use the normal form from mySQL ex: Area ASC,Date_insertPropertiesed DESC separated by commas'),
    '#default_value'  => variable_get('ram_property_orderby', 'Area ASC'),
    '#required'       => TRUE,
    '#weight'         => -46,
    '#group'          => 'filters'
 );
  
  $form['filters']['ram_property_date_inserted'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Date Inserted Key'),
    '#description'    => t('This is the token you wish to use to identify the key that stores the Date Inserted Data into to the system. ex: [Date_Inserted]'),
    '#default_value'  => variable_get('ram_property_date_inserted', '[Date_Inserted]'),
    '#required'       => TRUE,
    '#weight'         => -45,
    '#group'          => 'filters'
 );
  
  $form['filters']['ram_property_date_modified'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Date Modified Key'),
    '#description'    => t('This is the token you wish to use to identify the key that stores the Date Updated Data into to the system. ex: [Date_updatePropertiesd]'),
    '#default_value'  => variable_get('ram_property_date_modified', '[Date_Modified]'),
    '#required'       => TRUE,
    '#weight'         => -44,
    '#group'          => 'filters'
 );

  $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
  
  return $form;
  
}

/**
 * Set up content Associations
 */
function _ram_property_setup_content() {
  
  return propertyAdmin::_setupContentTypeForm();
  
}

/**
 * Set up first run
 */
function _ram_property_setup_firstrun() {
  
  return propertyAdmin::_setupFirstRunForm($requirements, '_ram_property_import_first_run');
  
}

/**
 * Set up batch image import
 */
function _ram_property_setup_import_images() {
  
  return propertyAdmin::_setupImagesImportForm($requirements, '_ram_property_import_images');
  
}

/**
 * Set up complete
 */
function _ram_property_setup_complete() {
  
  $form['cron'] = array(
    '#type' 				=> 'fieldset',
    '#title' 				=> t('Runtime Options'),
    '#collapsible' 	=> TRUE,
    '#collapsed' 		=> FALSE,
    '#weight'				=> -50,
    '#group'        => 'cron'
  );
  
  //Time Options
  $options = array(
    '00:00' => t('00:00 - 12:00 AM'),
    '01:00' => t('01:00 - 1:00 AM'),
    '02:00' => t('02:00 - 2:00 AM'),
    '03:00' => t('03:00 - 3:00 AM'),
    '04:00' => t('04:00 - 4:00 AM'),
    '05:00' => t('05:00 - 5:00 AM'),
    '06:00' => t('06:00 - 6:00 AM'),
    '07:00' => t('07:00 - 7:00 AM'),
    '08:00' => t('08:00 - 8:00 AM'),
    '09:00' => t('09:00 - 9:00 AM'),
    '10:00' => t('10:00 - 10:00 AM'),
    '11:00' => t('11:00 - 11:00 AM'),
    '12:00' => t('12:00 - 12:00 PM'),
    '13:00' => t('13:00 - 1:00 PM'),
    '14:00' => t('14:00 - 2:00 PM'),
    '15:00' => t('15:00 - 3:00 PM'),
    '16:00' => t('16:00 - 4:00 PM'),
    '17:00' => t('17:00 - 5:00 PM'),
    '18:00' => t('18:00 - 6:00 PM'),
    '19:00' => t('19:00 - 7:00 PM'),
    '20:00' => t('20:00 - 8:00 PM'),
    '21:00' => t('21:00 - 9:00 PM'),
    '22:00' => t('22:00 - 10:00 PM'),
    '23:00' => t('23:00 - 11:00 PM')
 );
  
  $form['cron']['ram_property_batch_cron_start_time'] = array(
    '#type'           => 'select',
    '#title'          => t('Schedule Batch Start Time'),
    '#options'        => $options,
    '#description'    => t('What time of the day should cron start batching? It is best to set this when there is little to no traffic on your site, or non-peak hours.'),
    '#default_value'  => variable_get('ram_property_batch_cron_start_time', '00:00'),
    '#required'       => TRUE,
    '#weight'         => -49,
    '#group'          => 'cron'
  );
  
  $form['cron']['ram_property_batch_cron_time_of_day_exclude'] = array(
    '#type'           => 'select',
    '#multiple'       => TRUE,
    '#title'          => t('Do Not Run Batches at this/these times'),
    '#options'        => $options,
    '#description'    => t('If you need times that cron should not run, you may select from these times to keep cron from running at peak hours.'),
    '#default_value'  => variable_get('ram_property_batch_cron_time_of_day_exclude', NULL),
    '#required'       => FALSE,
    '#weight'         => -48,
    '#group'          => 'cron'
 );
  
  //Run Time Batch Limits
  $options = array(
    10 => t(10),
    20 => t(20),
    30 => t(30),
  );
  
  $form['cron']['ram_property_batch_cron_limit'] = array(
    '#type'           => 'select',
    '#title'          => t('Maximum number of batches to run per cron run'),
    '#options'        => $options,
    '#description'    => t('How many batches should run per cron run? This can take a little trial and error to get the batches to run throughout the day.'),
    '#default_value'  => variable_get('ram_property_batch_cron_limit', 10),
    '#required'       => TRUE,
    '#weight'         => -47,
    '#group'          => 'cron'
 );
  
  // $expiration = strtotime('now +' .$staleness);
  $options = array(
    '3 days' => t('Three Days'),
    '7 days' => t('Seven Days'),
    '10 days' => t('Ten Days'),
    '30 days' => t('Thirty Days'),
    '90 days' => t('Ninety Days'),
 );
  
  $form['cron']['property_property_staleness'] = array(
    '#type'           => 'select',
    '#title'          => t('Permanent Deletion (Staleness)'),
    '#options'        => $options,
    '#description'    => t('How long should a node/property remain unpublished until it is permanently deleted?'),
    '#default_value'  => variable_get('property_property_staleness', '90 days'),
    '#required'       => TRUE,
    '#weight'         => -46,
    '#group'          => 'cron'
 );
  
  $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
  
  return $form;
  
}