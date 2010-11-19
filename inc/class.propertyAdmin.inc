<?php
/**
 * @name propertyAdmin
 * @category Module Administration.
 */
class propertyAdmin {
  
  /**
   * @param array $modules, a keyed array of modules added into the suite, these needed to be added in .install files
   * @return a themed status report for the module
   */
  public static function _getStatus() {
    
    // 1. Module Status
    $requirements = propertyAdmin::_getModuleStatus();
    $status = propertyAdmin::_themeStatusReport( $requirements );
    
    // 2. Cron Status
    $requirements = propertyAdmin::_getCronStatus();
    $status .= propertyAdmin::_themeStatusReport( $requirements );
    
    return $status;
    
  }
  
  /**
   * @return return a themed status table
   */
  public static function _getModuleStatus() {
    
    $requirements = array();
    
    //Global API Key requirement for geocoding
    $googleKey = variable_get( 'property_suite_map_key_google_geocoder', null );
    $yahooKey  = variable_get( 'property_suite_map_key_yahoo_geocoder', null );
    
    if ( empty( $googleKey ) || empty( $yahooKey ) ) { //Verify if the keys have been entered, if they have then flag both ak ok, if not give warnings every where
      
      $requirements['map_keys'] = array(
        'title'       => t( 'Property Suite API Keys have NOT been Entered properly.' ),
        'value'       => t( 'Module Configuration' ),
        'description' => t( 'you must enter in two separate keys for this module to work. One for the Google API (you can enter that at the gamp module) and one for the Yahoo API, which is the fallback in case we reach the limits imposed by either API.' ),
        'severity'    => REQUIREMENT_ERROR
      );
      
      drupal_set_message( t( 'Geocoding API Keys NOT Set! you must set the API keys for both Google Geocoding and Yahoo Geocoding for this module to work properly. Please proceed to Map API Keys, at: admin/settings/property_suite/settings' ), 'error' );

    } else {
      
      $MSG = t( 'Using: !api',
        array(
          '!api'     => ( variable_get( 'property_suite_disable_google_geocoding', NULL ) ) ? 'Yahoo Geocoding' : 'Google Geocoding'
        )
      );
      
      $requirements['map_keys'] = array(
        'title'       => t( 'Property Suite API Keys have been Entered properly.' ),
        'value'       => $MSG,
        'description' => t( 'You may change these keys by clicking on Map API Keys' ),
        'severity'    => REQUIREMENT_OK
      );
      
    }
    
    $modules = variable_get( 'property_suite_modules', null );
    
    if ( !empty( $modules) ) {
    
      foreach ( $modules as $module ) {
        
        if ( module_exists( $module['name'] ) && variable_get( $module['name'] .'_enabled', null ) == true ) {
          
          $module['severity'] = REQUIREMENT_OK;
          
        } else {
  
          $module['severity'] = REQUIREMENT_ERROR;
          
        }
        
        $requirements[$module['name']] = array(
          'title'       => t( $module['title'] ),
          'value'       => t( $module['value'] ),
          'description' => t( $module['description'] ),
          'severity'    => $module['severity']
        );
        
      }
    
    }
    
    return ( !empty( $requirements ) ) ? $requirements : null;
    
  }
  
  /**
   * @return return a themed status table
   */
  public static function _getCronStatus() {
    
    $requirements = array();
    
    $cronStats = variable_get( 'property_suite_cron_stats', null );
    
    if ( !empty( $cronStats ) ) {
    
      foreach ( $cronStats as $stat ) {
        
        $requirements[$stat['name']] = array(
          'title'       => t( $stat['title'] ),
          'value'       => t( $stat['value'] ),
          'description' => t( $stat['description'] ),
          'severity'    => $stat['severity']
        );
        
      }
    
    } else {
      
      $requirements['cron_stats_empty'] = array(
        'title'       => t( 'Property Suite Cron Run Statistics' ),
        'value'       => t( 'Cron Run Setup' ),
        'description' => t( 'Cron has not run on this site, this message will change when cron runs, and there is a message to report back from the suite modules.' ),
        'severity'    => REQUIREMENT_INFO
      );
      
    }
    
    return ( !empty( $requirements ) ) ? $requirements : null;
    
  }
  
  /**
   * @param array $cronStat sets the variable 'property_suite_cron_stats' with a populated array that shows the current status of anything you wish to alert
   */
  public static function _setCronStatus( $cronStat ) {
    
    if ( is_array( $cronStat ) && !empty( $cronStat ) ) {
      
      $cronStats = variable_get( 'property_suite_cron_stats', null );
      
      $name = $cronStat['name'];
      unset( $cronStats[$name] );
      $cronStats[$name] = $cronStat;
      
      variable_set( 'property_suite_cron_stats', $cronStats );
    
    }
    
  }
  
  /**
   * @param array $requirements = an array of requirements that conforms to drupal's $requirements array
   */
  public static function _themeStatusReport( $requirements ) {
    
    if ( is_array( $requirements ) ) {
      
      return theme( 'status_report', $requirements );
    
    }
    
  }
  
  /**
   * @param array $module an array to add to the variable property_suite_modules
   * @param string $module['foo']['name'] = the machine name of the module
   * @param string $module['foo']['title'] = the title you gave the module
   * @param string $module['foo']['value'] = any relevent value you need to give, most likely a version number or other information
   * @param string $module['foo']['description'] = the description you wish to give the module you are loading.
   */
  public static function _registerModule( $module ) {
    
    if ( is_array( $module ) ) { //Register the array in the property suite, and make sure that the weight of the next module is higher than the last
    
      $modules = variable_get( 'property_suite_modules', array() );
      
      $modules = array_merge( $module, $modules );
      
      variable_set( 'property_suite_modules', $modules );
      
      $weight = db_result( db_query( "SELECT weight FROM {system} WHERE name = 'property' " ) ) + 1;
      
      db_query("UPDATE {system} SET weight = %d WHERE name = '%s'", $weight, $module[key($module)]['name'] );
      
      drupal_set_message( t( "!name has been registered with the Property Suite module.", array( '!name' => $module[key($module)]['title'] ) ), 'status' );
      
      menu_rebuild(); //Rebuild menues when a module registers
    
    }
    
  }
  
  /**
   * @param string $module = the module name you wish to unset
   */
  public static function _unregisterModule( $module ) {
    
    if ( is_string( $module ) ) {
    
      $modules = variable_get( 'property_suite_modules', array() );
      
      unset( $modules[$module] );
      
      variable_set( 'property_suite_modules', $modules );
      
      drupal_set_message( t( "!name has been unregistered from the Property Suite module.", array( '!name' => $module ) ), 'status' );
      
      menu_rebuild(); //Rebuild menues when a module registers, and un registers
    
    }
    
  }
  
  /**
   * @name _setupCacheForm()
   * return array the form API array for creating caches
   */
  
  public static function _setupCacheForm( $requirements = null ) {
    
    $form['cache'] = array(
      '#type' 				=> 'fieldset',
      '#title' 				=> t('Manually Cache'),
      '#collapsible' 	=> TRUE,
      '#collapsed' 		=> FALSE,
      '#weight'				=> -47,
      '#group'        => 'runtime'
    );
    
    $form['cache']['property_cache_all'] = array(
      '#type'         => 'submit',
      '#value'        => t( 'Cache All' ),
      '#prefix'       => '<p><div id="cache-progress"></div></p><p>',
      '#suffix'       => t( 'Press this button to manually cache all data to the site.' ) .'</p>',
      '#required'     => true,
      '#weight'       => -49,
      '#group'        => 'runtime'
    );
  
    $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
    
    return $form;
    
  }
  
  /**
   * @name _setupTokenForm()
   * return array the form API array for creating tokens
   */
  
  public static function _setupTokenForm() {
    
    $form['tokens_and_keys'] = array(
      '#type' 				=> 'fieldset',
      '#title' 				=> t('Tokens'),
      '#collapsible' 	=> TRUE,
      '#collapsed' 		=> FALSE,
      '#weight'				=> 49,
      '#group'        => 'tokens_and_keys'
    );
    
    $form['tokens_and_keys']['property_tokens'] = array(
      '#type'           => 'textarea',
      '#rows'           => 20,
      '#title'          => t( 'Token Associations' ),
      '#description'    => t( 'The proprty API uses associative keys, which translates a key to a token value. This allows the api to write out queries, etc from tokenized values, which will change as your implelentation changes. Please separate keys by a return, in the form key|association. ' ),
      '#default_value'  => variable_get( 'property_tokens', null ),
      '#required'       => true,
      '#weight'         => -49,
      '#group'          => 'tokens_and_keys'
    );
    
    $form['tokens_and_keys']['property_connection_tokenize'] = array(
      '#type'         => 'submit',
      '#value'        => t( 'Generate Tokens' ),
      '#prefix'       => '<div id="token-progress"></div><p>',
      '#suffix'       => t( 'After you have made sure that you want to use this set of keys and labels, press Generate Tokens to generate the tokens. You may also press save configuration, to just save the keys.' ) .'</p>',
      '#required'     => true,
      '#weight'       => -48,
      '#group'        => 'tokens_and_keys'
    );
    
    if ( variable_get( 'property_connection_tokenized', null ) === true ) {
        
      $form['tokens_and_keys']['token_help'] = array(
        '#value'  => theme('token_help', 'property' ),
        '#weight' => -47,
        '#group'  => 'tokens_and_keys'
      );
    
    }
  
    $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
    
    return $form;
    
  }
  
  /**
   * @name _setupContentTypeForm()
   * return array the form API for this form, so other modules can share it
   * @todo = make this more API like
   */
  public static function _setupContentTypeForm() {
    
    if ( variable_get( 'property_content_type_last', null ) != variable_get( 'property_content_type', null ) ) {
      
      //start by destroying all the variables set if we have no property type, or we have changed our property content type.
      foreach ( variable_get( 'property_content_type_field_names', array() ) as $variable ) {
        
        variable_del( $variable );
        
      }
      
      variable_set( 'property_content_type_last', variable_get( 'property_content_type', null ) );
    
    }
    
    $form['property_content_type'] = array(
      '#type' 				=> 'fieldset',
      '#title' 				=> t('Property Content Type'),
      '#collapsible' 	=> TRUE,
      '#collapsed' 		=> FALSE,
      '#weight'				=> 47,
      '#group'        => 'property_content_type'
    );
    
    $options = node_get_types( 'names' );
    unset( $options['webform'] ); //Since the webform module is really not supported, kill this.
    $form['property_content_type']['setup']['property_content_type'] = array(
      '#type'           => 'select',
      '#title'          => t( 'Chose your Content Type.' ),
      '#description'    => t( 'Chose the content type you wish to map an association to. After you select this cotent type, please press save configuration and the system will lock your content type, and start the mapping process.' ),
      '#options'        => $options,
      '#default_value'  => variable_get( 'property_content_type', null ),
      '#required'       => true,
      '#weight'         => -49,
      '#group'          => 'property_content_type'
    );
    
    if ( variable_get( 'property_content_type', null ) != null ) { //get the options available to map.
      
      $tokens_raw = token_get_list( 'property' );
      unset( $tokens_raw['global'] );
      
      $tokens = array( 'NULL' => 'No Mapping Assigned' );
      foreach ( $tokens_raw['Property Suite'] as $key=> $value ) {
        
        $tokens[trim($key)] = trim($key);
        
      }
      
      //@todo - this variable cannot have RAM in it!!
      $tokens[variable_get( 'ram_property_image_key', 'imagesavailable' )] = 'Image_Field'; //Add imagefield support @todo add in File Field support for attachments - This needs to be left at the end, because it is how I pull it from the array, using end()
      asort( $tokens );
      $type = content_types( variable_get( 'property_content_type', null ) );
      
      $name = 'property_content_type_' .variable_get( 'property_content_type', null ) .'_node_title';
      
      $variable = array( $name );
      
      $form['property_content_type']['setup'][0][ $name ] = array( //set up the node title, since this is always there
        '#type'           => 'textfield',
        '#title'          => t( 'Node Title' ),
        '#default_value'  => variable_get( $name, NULL ),
        '#description'    => t( 'You can add any tokens together to create a title for this node, all spaces and non-ascii characters will automatically replace with the replacement character you select above. ex: [Address]_[City]_[State]_[Zip]' ),
        '#required'       => TRUE,
        '#weight'         => -48,
        '#group'          => 'property_content_type'
      );
      
      $options = array(
        '#HYPHEN' => t( 'Hyphen - ' ),
        '#UNDERSCORE' => t( 'Underscore _ ' ),
      );
      $form['property_content_type']['setup'][0]['property_content_type_token_ascii_replacement'] = array(
        '#type'           => 'select',
        '#title'          => t( 'Replace ASCII Characters.' ),
        '#description'    => t( 'You may chose from a hyphen (-) or underscore (_) as these are the best for SEO purposes.' ),
        '#options'        => $options,
        '#default_value'  => variable_get( 'property_content_type_token_ascii_replacement', '#HYPHEN' ),
        '#required'       => TRUE,
        '#weight'         => -47,
        '#group'          => 'property_content_type'
      );
      
      $form['property_content_type']['setup'][0]['token'] = array(
        '#type' 				=> 'fieldset',
        '#title' 				=> t('Property Node Tokens'),
        '#collapsible' 	=> TRUE,
        '#collapsed' 		=> TRUE,
        '#weight'				=> -100,
        '#group'        => 'property_content_type'
      );
      
      $form['property_content_type']['setup'][0]['token']['tokens'] = array(
        '#value'  => theme('token_help', 'property' ),
        '#weight' => -50,
        '#group'  => 'property_content_type'
      );
      
      //Location fields
      if ( module_exists( 'location' ) ) {
        
        $location_tokens = variable_get( 'location_settings_node_' .variable_get( 'property_content_type', null ), array() ); //location fields enabled
        
        unset( $location_tokens['form']['fields']['name'] ); //Unset the name fields, since it is hard to support.
        unset( $location_tokens['form']['fields']['locpick'] ); //Unset the locpick option, because it is not necessary here.
        
        if ( !empty( $location_tokens ) ) {
          
          foreach ( $location_tokens['form']['fields'] as $key => $value ) {
            
            if ( $value['collect'] == 1 ) { //if it is set to collect this value, show it
              
              $name = 'property_content_type_' . variable_get( 'property_content_type', null ) .'_location_' .$key;
              $variable[] = $name;

              $exclude = array('street','additional');
              if (!in_array($key,$exclude)) {
                $form['property_content_type']['setup'][1][ $name ] = array(
                  '#type'           => 'select',
                  '#title'          => t( 'Location !name', array( '!name' => ucwords( $key ) ) ),
                  '#options'        => $tokens,
                  '#default_value'  => variable_get( $name, 'NULL' ),
                  '#required'       => TRUE,
                  '#weight'         => $value['weight'],
                  '#group'          => 'property_content_type',
                );
              } else {
                $form['property_content_type']['setup'][1][ $name ] = array(
                  '#type'           => 'textfield',
                  '#title'          => t( 'Location !name', array( '!name' => ucwords( $key ) ) ),
                  '#default_value'  => variable_get( $name, NULL ),
                  '#description'    => t('You can add any tokens together to create a geocodable address for this node, ex: [Address]'),
                  '#required'       => TRUE,
                  '#weight'         => $value['weight'],
                  '#group'          => 'property_content_type',
                );
              }
              
            }
            
          }
          
        }
        
      }
      
      $fields = $type['fields'];
      
      foreach ( $fields as $field ) { //for each fields output the fieldname and a select list for them to assign it to
        
        $name = 'property_content_type_' .variable_get( 'property_content_type', null ) .'_' .$field['field_name'] ;
        $variable[] = $name;
        
        $form['property_content_type']['setup'][2][ $name ] = array(
          '#type'           => 'select',
          '#title'          => $field['field_name'],
          '#options'        => $tokens,
          '#default_value'  => variable_get( $name, 'NULL' ),
          '#weight'         => $field['widget']['weight'],
          '#group'          => 'property_content_type'
        );
        
      }
      
      variable_set( 'property_content_type_field_names', $variable ); //regen field names on the fly
      
    }
    
    $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
    
    return $form;
    
  }
  
  /**
   * @name _setupFirstRunForm()
   * @param array $requirements = you should use the array $requirements['batch'] to pipe in the proper requirements
   * return array the form API array for creating first run
   */ 
  public static function _setupFirstRunForm( $requirements = null, $function = NULL ) {
    
    $form['runtime'] = array(
      '#type' 				=> 'fieldset',
      '#title' 				=> t('Manually Import'),
      '#collapsible' 	=> TRUE,
      '#collapsed' 		=> FALSE,
      '#weight'				=> -49,
      '#group'        => 'runtime'
    );
    
    $totals = propertyTrans::_getTotals(); //Get the totals from the last cache call
    $insertBatchImages = propertyCache::_getCache( 'property_cache_insert_images' );
    $updateBatchImages = propertyCache::_getCache( 'property_cache_update_images' );
    
    if ( $totals['chunks'] > 0 && empty( $insertBatchImages ) && empty( $updateBatchImages ) ) {
      
      $i = 0;
      while ( $i++ < $totals['chunks'] ) {
        
        $options[$i] = $i;
        
      }
      
      $form['runtime']['property_batch_chunk_maximum_limit'] = array(
        '#type'           => 'select',
        '#title'          => t( 'Maximum Limit of Chunks to Process in this import.' ),
        '#options'        => $options,
        '#description'    => t( 'Set the limit of chunks that you wish to execute for a given batch. Defaults to 1, so you can do a small import first. Please press "Save configuration" to update this variable, before you start the batch.' ),
        '#default_value'  => variable_get( 'property_batch_chunk_maximum_limit', 1 ),
        '#required'       => true,
        '#weight'         => -49,
        '#group'          => 'runtime'
      );
      
      $form['runtime']['property_batch_all'] = array(
        '#type'         => 'submit',
        '#value'        => t( 'Manually Batch Import' ),
        '#submit'       => array( $function ),
        '#prefix'       => '<p>',
        '#suffix'       => t( 'Press this button to manually batch your import, this may take some time to complete, as the system has auto calculated the number of nodes per batch. Make sure that you have your content type mapped properly, and then wait for this import to complete. You cannot undo this action once it starts, so back up first!' ) .'</p>',
        '#required'     => true,
        '#weight'       => -49,
        '#group'        => 'runtime'
      );
      
    } else {
      
      $form['runtime']['MSG'] = array(
        '#value' => t( 'There are no batches to process, or you must batch process all the images into the system first.' ),
        '#group' => 'runtime',
      );
    
    }
  
    $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
    
    return $form;
    
  }
  
  /**
   * @name _setupImagesImportForm()
   * @param array $requirements = you should use the array $requirements['batch'] to pipe in the proper requirements
   * return array the form API array for creating first run
   */ 
  public static function _setupImagesImportForm( $requirements = null, $function = NULL ) {
    
    $form['runtime_images'] = array(
      '#type' 				=> 'fieldset',
      '#title' 				=> t('Manually Import Images'),
      '#collapsible' 	=> TRUE,
      '#collapsed' 		=> FALSE,
      '#weight'				=> -48,
      '#group'        => 'runtime'
    );
    
    $totals = propertyTrans::_getTotalsImages(); //Get the totals from the last cache call
    
    if ( $totals['chunks'] > 0 ) { //if we have chunks, allow for manual import. '_ram_property_import_images'
      
      $i = 0;
      while ( $i++ < $totals['chunks'] ) {
        
        $options[$i] = $i;
        
      }
      
      $form['runtime_images']['property_batch_chunk_maximum_limit_images'] = array(
        '#type'           => 'select',
        '#title'          => t( 'Maximum Limit of Chunks to Process in this import.' ),
        '#options'        => $options,
        '#description'    => t( 'Set the limit of chunks that you wish to execute for a given batch. Defaults to 1, so you can do a small import first. Please press "Save configuration" to update this variable, before you start the batch.' ),
        '#default_value'  => variable_get( 'property_batch_chunk_maximum_limit_images', 1 ),
        '#required'       => true,
        '#weight'         => -49,
        '#group'          => 'runtime_images'
      );
      
      $form['runtime_images']['property_batch_all'] = array(
        '#type'         => 'submit',
        '#value'        => t( 'Manually Batch Import Images' ),
        '#submit'       => array( $function ),
        '#prefix'       => '<p>',
        '#suffix'       => t( 'Press this button to manually batch your images, this may take some time to complete, as the system has auto calculated the number of nodes per batch. Make sure that you have your content type mapped properly, and then wait for this import to complete. You cannot undo this action once it starts, so back up first!' ) .'</p>',
        '#required'     => true,
        '#weight'       => -49,
        '#group'        => 'runtime_images'
      );
      
    } else {
      
      $form['runtime_images']['MSG'] = array(
        '#value' => t( 'No Batches To Process' ),
        '#group' => 'runtime_images',
      );
    
    }
  
    $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
    
    return $form;
    
  }
  
}