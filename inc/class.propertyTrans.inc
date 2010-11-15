<?php
/**
 * @name propertyTrans
 * @category Module Transactions.
 */
class propertyTrans { //propertyTrans handles all transactions in the Property Suite Module, this is an abstract class so we can use child classes''
  
  public $properties;
  
  /**
   * @name _loadBroker() = loads a broker user, when given a broker id
   */
  public static function _loadBroker( $agentID = null ) {

    $uid = 0;
    $uid = db_result( db_query( "SELECT uid FROM {profile_values} WHERE value = %d", trim( $agentID ) ) );
    
    if ( $uid > 1 ) {
      
      return user_load( (int) $uid );
      
    } else {
      
      return user_load( 1 ); //Let's make sure everything get's under the super user if it has not broker id.
      
    }
    
  }
  
  /**
   * @name _nodeImage( $node = null, $field = null, $image = null ) = curls the image to the local server
   */
  public static function _nodeImage( $node = null, $field = null, $image = null ) {
        
    $localPath  = PROPERTY_MODULE_UPLOAD_DIRECTORY;
    $localImage = $localPath.$image;

    $fid = db_result( db_query( "SELECT fid FROM {files} WHERE filepath LIKE '%s'", $localImage ) ); //So, if the file is already added into the database, do not add it again?

    if ( $field && file_exists( $localImage ) && !$fid ) { //If the file is already in the directory from the cache call, then add it to the node array after getting an fid
      
      $imageInfo = image_get_info( $localImage ); //If this does not show up, then it is because the image is broken or corrupted
      //print_r( $localImage );

      if ( !empty( $imageInfo ) ) {
        
        $imageNameInfo = explode( '/', $localImage );
        $imageName     = end( $imageNameInfo );

        $file = array(
          'uid'       => 1,
          'filename'  => $imageName,
          'filepath'  => $localImage,
          'filemime'  => $imageInfo['mime_type'],
          'filesize'  => $imageInfo['file_size'],
          'status'    => FILE_STATUS_PERMANENT,
          'timestamp' => time(),
        );
        
        drupal_write_record( 'files', $file ); //this can get out of control really fast, I wish there was a better way to add to the files db with some sort of better check
        
        if ( !is_array( $node->$field ) ) { //For some reason, this loses it's array here
          
          $node->$field = array();
          
        }
        
        array_push( $node->$field, array( 'fid' => $file['fid'], 'list' => true, 'data' => null ) );
        
        return $node;
        
      } else {
        
        return $node;
        
      }
      
    } else { //If there is an FID return the node, since it is already in here
      
      return $node;
      
    }
    
  }
  
  public static function _nodeLocation( $node, $location ) { //@todo There is a way to make this add multiple locations, I'm just missing this right now
    
    $settings = variable_get( 'location_settings_node_' .variable_get( 'property_content_type', null ), null );
    
    if ( $settings ) { //If there are no settings for this node, don't return anything but the node object
      
      $location['location_country'] = ( empty( $location['location_country'] ) ) ? $settings['form']['fields']['country']['default'] : $location['location_country'];
      
      //geocode here, so we don't have update problems
      $geocode = self::_geoCodeProperty( $node, $location );
      
      $locations = array (
        array (
          'street'      => t( $location['location_street'] ),
          'additional'  => t( $location['location_additional'] ),
          'city'        => t( $location['location_city'] ),
          'province'    => t( location_province_code( $location['location_country'], $location['location_province'] ) ),
          'postal_code' => t( $location['location_postal_code'] ),
          'country'     => t( $location['location_country'] ), //get the default value!
          'latitude'    => $geocode['lat'],//geocode the longitude now
          'longitude'   => $geocode['lon'],//geocode the latitude now
          'phone'       => t( $location['location_phone'] ),
          'fax'         => t( $location['location_fax'] ),
        )
      );
   
      $node->locations = $locations;
      
    }
    
    usleep(100000); //sleep the system, when geocoding - to make sure that google does not report a 620 limit error. They will ban you for 24hours if you don't
    
    return $node;
    
  }
  
  /**
   * @name _geoCodeProperty( $node, $location )
   * @param array $location - the location object from the location module
   * @return array $geocode = an Array with the geocoded latitude and longitude of this address
   */
  public static function _geoCodeProperty( $node, $location ) {
    
    $geocode = array();
      
    $key = variable_get('location_geocode_google_apikey', '');
    
    if ( function_exists('gmap_get_key') ) {
      
      $key = gmap_get_key();
      
    }
    
    //translate location to something google geocode can understand
    $street      = $location['location_street'];
    $city        = $location['location_city'];
    $province    = $location['location_province'];
    $postal_code = $location['location_postal_code'];
    
    $disableGoogle = variable_get( 'property_suite_disable_google_geocoding', FALSE );
    
    /**
     * Google API
     */
    $status = NULL;
    if ( !$disableGoogle ) { //If google is not disabled, or the elapsed time is more than 24 hours, then you can re-set to google
      
      //parse the data and get a response
      $data  = urlencode( "$street $city, $province, $postal_code" );
      $q     = $data;
      $url   = 'http://maps.google.com/maps/geo?output=xml&key='. $key .'&q=' .$q;
      $reply = drupal_http_request( $url );
      //print_r( $reply );
      $xml   = propertyTrans::_validateData( $reply->data );
      
      //Turn that data into the proper location array that location module understands - GOOGLE GEOCODER STATUS
      $status = (int) $xml->Response->Status->code;
          
      $geocodeData = (string) $xml->Response->Placemark->Point->coordinates; //Since we get a valid XML string back from validate, we can just traverse the tree
      $geocodeData = explode( ',', $geocodeData ); //explode on the comma
      
      $geocode['lon'] = $geocodeData[0]; //position 0 is the longitude
      $geocode['lat'] = $geocodeData[1]; //position 1 is the latitude
      
      $provider = t( 'Google' );
      
    }
    
    if ( $status == 620 && $provider == 'Google' ) { //If we have a status that stops google, then flag it
      
      variable_set( 'property_suite_disable_google_geocoding', time() ); //Set this to disable for 24 hours, and use Yahoo geocoding then
      
    }
    
    /**
     * Yahoo API
     */
    
    if ( $disableGoogle || !is_array( $geocodeData ) ) { //Fall Back to using Yahoo's geocoding service, this guarantees we can have upto 15,0000 calls a day between the two

      $key   = variable_get( 'property_suite_map_key_yahoo_geocoder', null );
      $url   = 'http://local.yahooapis.com/MapsService/V1/geocode?appid='. $key .'&street=' .urlencode( $street ) .'&city=' .urlencode( $city ) .'&state=' .urlencode( $province ) .'&zip=' .urlencode( $postal_code );
      $reply = drupal_http_request( $url );
      //print_r( $reply );
      $xml   = propertyTrans::_validateData( $reply->data );
      
      $geocode['lat'] = (string) $xml->Result->Latitude;
      $geocode['lon'] = (string) $xml->Result->Longitude;
      
      $provider = t( 'Yahoo' );
    
    }
    
    $msg = t( 'Property Module Geocoded: !title at with Location: LAT !lat LON !lon.', array( '!title' => $node->title, '!lat' => $geocode['lat'], '!lon' => $geocode['lon'] ) );
    if ( empty($geocode['lat'] ) || empty( $geocode['lon'] ) ) {

      $status = REQUIREMENT_ERROR;
      $msg    = t( 'Could Not Geocode: !title', array( '!title' => $node->title ) );
      
    }
    
    $logs['Geocode'][] = array(
      'title'       => t( 'Property' ),
      'description' => $msg,
      'severity'    => $status,
      'value'       => $provider,
    );
  
    self::_logMessages( $logs ); //Log The messages
    
    //return the array back to the location module
    return $geocode;
    
  }
  
  public static function _nodeFields( $node ) {
    
    $contentFields = content_fields( null, variable_get( 'property_content_type', null ) ); //This is kinda unreliable, but I don't have another option
    
    if ( !empty( $contentFields ) ) {
      
      foreach ( $contentFields as $key => $value ) {
        
        if (  $value['type_name'] == variable_get( 'property_content_type', null ) ) {
          
          //turn fields into node objects for save @todo keep adding to this array for date values, and major widgets
          switch ( $value['type'] ) {
            
            case 'text': //This is the core cck field
            case 'filefield': //This is from the file field module
            case 'integer':
            case 'float':
            default:
              
              $node->$value['field_name'] = array();
              
            break;
            
          }
          
        }
        
      }
      
      unset( $contentFields );
      
    }
    
    return $node;
    
  }
  
  public static function _nodePrepare( $title = null ) {
    
    $node           = new stdClass();
    $node->title    = t( str_replace( array( "&", "&amp;" ), array( "And", "And" ), $title ) );
    $node->body     = null;
    $node->type     = variable_get( 'property_content_type', null );
    $node->created  = time();
    $node->changed  = $node->created;
    $node->status   = 1;
    $node->promote  = 0;
    $node->sticky   = 0;
    $node->format   = 2;//default format?
    $node->language = 'en';
    
    $node = self::_nodeFields( $node );
    
    return $node;
    
  }
  
  /**
   * @name _nodeSave
   * @param object $data a valid
   * @ return FALSE || $log
   */
  public static function _nodeSave( $node, $agent, $propertyKeyValue, $op = 'Insert' ) { //Save that node!
        
    if ( strlen( $node->title ) > 0 && !empty( $propertyKeyValue ) ) { //For some reason, bad node titles need to be expunged
      
      $node->uid  = $agent->uid; //assigne the broker uid to the user, save, then unload
      $node->name = $agent->name;
      
      if ( is_object( $node ) ) {
        
        $errors = false; //I have a feeling I have to clear this every time
        node_validate( $node ); //This is just to report back to us if there are validation errors
        $errors = form_get_errors();
        $node   = node_submit( $node ); //this does it's job, but always frakin returns true
        
        if ( $node->validated == 1 ) { //If the node is validated, run save
            
          node_save( $node ); //Leave me here, or I don't validate
          
          if ( is_numeric( $node->nid ) && !empty( $node->nid ) ) { //If we have a valid nid and no errors, push the update
            
            if ( module_exists( 'location' ) && empty( $node->locations ) ) { //Save the location if there is no location added
              
              location_save_locations ( $node->locations, array ('nid' => $node->nid, 'vid' => $node->vid ) );
              
            }
            
            $properties                    = ( is_array( $importedProperties ) ) ? $importedProperties : array(); // if the imported properties are not an array, make it one
            $properties[$propertyKeyValue] = $node->nid;
            
            $logs['Save'][] = array(
              'title'       => t( 'Property' ),
              'description' => t( 'Property Module Saved Node !title at !link.', array( '!link' => l( 'View This Node', 'node/' .$node->nid ), '!title' => $node->title  ) ),
              'severity'    => REQUIREMENT_OK,
              'value'       => $op,
            );

          }
          
          if ( $errors ) { //If there are errors, let's log them here
            
            $logs['Validate'][] = array(
              'title'       => t( 'Property' ),
              'description' => t( 'Property Module Could Not Saved Node !title with nid !nid because of validation errors: !errors.', array( '!title' => $node->title, '!nid' => $node->nid, '!errors' => print_r( $errors, TRUE ) ) ),
              'severity'    => REQUIREMENT_ERROR,
              'value'       => 'Node Validation - ' .$op,
            );
            
          }

        } else {
          
          $logs['Validate'][] = array(
            'title'       => t( 'Property' ),
            'description' => t( 'Property Module Could not Validate: !title.', array( '!title' => $node->title ) ),
            'severity'    => REQUIREMENT_ERROR,
            'value'       => 'Node Validation',
          );
          
        }
        
        unset( $node );
        self::_logMessages( $logs ); //Log The messages
    
        return TRUE;
        
      } else {
        
        unset( $node );
        
        return FALSE;
        
      }
      
    }
  
  }
  
  /**
   * @name _logMessages( $logs )
   * Caches Messages logged into the system so they can be added into a list
   */
  public static function _logMessages( $logs = NULL ) {
    
    if ( is_array( $logs ) ) {
      
      $log = ( is_array( propertyCache::_getCache( 'property_suite_transaction_logs' ) ) ) ? propertyCache::_getCache( 'property_suite_transaction_logs' ) : array() ;
      
      $new_logs = array_merge_recursive($log,$logs);
      propertyCache::_setCache( 'property_suite_transaction_logs', $new_logs, CACHE_PERMANENT, 'cache_property_suite' );
      unset( $logs );
      
    }
    
  }
  
  /**
   * @name _processMessages()
   * Processes the messages cached into the system
   */
  public static function _processMessages() {
    
    $logs = propertyCache::_getCache( 'property_suite_transaction_logs' );
    
    if ( $logs && is_array( $logs ) ) { //If we have watchdog logs and they area an array run it
      
      foreach ( $logs as $key => $log ) {
        
        $data = propertyAdmin::_themeStatusReport( $log );
        
        watchdog( t( 'Property !type', array( '!type' => $key ) ), 'Property Suite Information Log on !time !details', array( '!time' => date( 'm-d-Y h:i:s A', strtotime( "now" ) ), '!details' => $data ), WATCHDOG_INFO, 'System Log' ); //watchdog log

      }
      
      return $data;
      
    }
    
  }
  
  /**
   * @name _createPropertyBatch
   * @param object $manager = the manager object you wish to run the _insertProperties and _updateProperties methods on
   * @return = no return value
   * This function creates a batch for you to initiate from your fooManager class. This sets the batch from your caches you created,
   * and runs a sequential import and update based on your filters.
   * Since these are admin functions, they have been relegated to the admin.property.inc file
   */
  public static function _createPropertyBatch( $manager, $limit = 0 ) {
    
    if ( !ini_get('safe_mode') ) { //Set the ini time, scammed this from devel
      
      set_time_limit(240);
      
    }
    
    $insertBatch = propertyCache::_getCache( 'property_cache_insert_items' );

    $updateBatch = propertyCache::_getCache( 'property_cache_update_items' );
    
    $operations = array();
    
    if ( variable_get( 'property_batch_chunk_maximum_limit', null ) == 0 ) { //If for some reason we are getting a zero value, then set it to 1 so at least we can run the update
      
      variable_set( 'property_batch_chunk_maximum_limit', 1 );
      
    }
    
    $limit = ( $limit == 0 ) ? variable_get( 'property_batch_chunk_maximum_limit', 1 ) : $limit; //switch the limit, if we override the chunk limt via cron
    
    if ( is_array( $insertBatch ) ) { //if the insert batch is not empty, run it
      
      foreach ( $insertBatch as $key => $value ) {
        
        if ( count( $operations ) < $limit ) { //This is not too elegant
          
          $batchMSG[] = t( 'Batch Insert %key : %count items', array( '%key' => $key, '%count' => count( $value ) ) );
          
          $operations[] = array( '_property_suite_admin_batch_insert_properties', array( $key, $manager ) );
          
        }
        
      }
      
      unset( $insertBatch );
      
    }
    
    if ( is_array( $updateBatch ) ) { //if the update batch is not empty, run it
      
      foreach ( $updateBatch as $key => $value ) {
        
        if ( count( $operations ) < $limit ) { //This is not too elegant, and repeated
          
          $batchMSG[] = t( 'Batch Update %key : %count items', array( '%key' => $key, '%count' => count( $value ) ) );
        
          $operations[] = array( '_property_suite_admin_batch_update_properties', array( $key, $manager ) );
          
        }
        
      }
      
      unset( $updateBatch );
      
    }
    
    // Set up the Batch API
    $batch = array(
      'operations'       => $operations,
      'finished'         => '_property_suite_admin_batch_properties_finished',
      'file'             => PROPERTY_MODULE_BATCH_FILE_PATH,
      'title'            => t('Importing And Updating Properties'),
      'init_message'     => t('Starting Import and Update, this may take a few minutes to gather all the data. Summary of Batch Info !batch', array( '!batch' => '<br />' .implode( '<br />', $batchMSG ) ) ),
      'progress_message' => t('Importing/Updating @current out of @total batches.'),
      'error_message'    => t('Sorry, an error occurred, this is a fatal error, please check the recent log entries for details and try again.'),
    );
    
    batch_set( $batch ); //set our batch, and watch it run
    batch_process( $manager->batchProcessPath );
    $manager->__destruct();
    unset( $operations );
    
  }
  
  /**
   * @name _createImageBatch
   * @param object $manager = the manager object you wish to run the _insertProperties and _updateProperties methods on
   * @return = no return value
   * This function creates a batch for you to initiate from your fooManager class. This sets the batch from your caches you created,
   * and runs a sequential import and update based on your filters.
   * Since these are admin functions, they have been relegated to the admin.property.inc file
   */
  public static function _createImageBatch( $manager, $limit = 0 ) {
    
    if ( !ini_get('safe_mode') ) { //Set the ini time, scammed this from devel
      
      set_time_limit(240);
      
    }
    
    $fileTime = propertyCache::_getCache( 'property_cache_filetimes' );
    
    $insertBatch = propertyCache::_getCache( 'property_cache_insert_images' );

    $updateBatch = propertyCache::_getCache( 'property_cache_update_images' );
    
    $operations = array();
    
    if ( variable_get( 'property_batch_chunk_maximum_limit', null ) == 0 ) { //If for some reason we are getting a zero value, then set it to 1 so at least we can run the update
      
      variable_set( 'property_batch_chunk_maximum_limit', 1 );
      
    }
    
    $limit = ( $limit == 0 ) ? variable_get( 'property_batch_chunk_maximum_limit_images', 1 ) : $limit; //switch the limit, if we override the chunk limt via cron
    
    if ( is_array( $insertBatch ) ) { //if the insert batch is not empty, run it
      
      foreach ( $insertBatch as $key => $value ) {
        
        if ( count( $operations ) < $limit ) { //This is not too elegant
          
          $batchMSG[] = t( 'Batch Insert %key : %count images', array( '%key' => $key, '%count' => count( $value ) ) );
          
          $operations[] = array( '_property_suite_admin_batch_insert_images', array( $key, $manager, $fileTime ) );
          
        }
        
      }
      
      unset( $insertBatch );
      
    }
    
    if ( is_array( $updateBatch ) ) { //if the update batch is not empty, run it
      
      foreach ( $updateBatch as $key => $value ) {
        
        if ( count( $operations ) < $limit ) { //This is not too elegant, and repeated
          
          $batchMSG[] = t( 'Batch Update %key : %count images', array( '%key' => $key, '%count' => count( $value ) ) );
        
          $operations[] = array( '_property_suite_admin_batch_update_images', array( $key, $manager, $fileTime ) );
          
        }
        
      }
      
      unset( $updateBatch );
      
    }
    
    // Set up the Batch API
    $batch = array(
      'operations'       => $operations,
      'finished'         => '_property_suite_admin_batch_images_finished',
      'file'             => PROPERTY_MODULE_BATCH_FILE_PATH,
      'title'            => t('Importing And Updating Images'),
      'init_message'     => t('Starting Import and Update, this may take a few minutes to gather all the data. Summary of Batch Info !batch', array( '!batch' => '<br />' .implode( '<br />', $batchMSG ) ) ),
      'progress_message' => t('Importing/Updating @current out of @total batches.'),
      'error_message'    => t('Sorry, an error occurred, this is a fatal error, please check the recent log entries for details and try again.'),
    );
    
    batch_set( $batch ); //set our batch, and watch it run
    batch_process( $manager->batchProcessPath );
    $manager->__destruct();
    unset( $operations );
    unset( $fileTime );
    
  }
  
  /**
   * @name _createShellBatchProperties
   * @param object $manager = the manager object you wish to run the _insertProperties and _updateProperties methods on
   * @return = no return value
   * This function creates a batch for you to initiate from your .sh file. This is meant to be the back end cron run, and requires a re-factoring to make all this work
   */
  public static function _createShellBatchProperties( $manager, $limit = 0 ) {
    
    if ( !ini_get('safe_mode') ) { //Set the ini time, scammed this from devel
      
      set_time_limit(240);
      
    }
    
    $insertBatch = propertyCache::_getCache( 'property_cache_insert_items' );
    //print_r( $insertBatch );
    $updateBatch = propertyCache::_getCache( 'property_cache_update_items' );
    //print_r( $updateBatch );
    
    $i = 0;
    
    if ( variable_get( 'property_batch_chunk_maximum_limit', null ) == 0 ) { //If for some reason we are getting a zero value, then set it to 1 so at least we can run the update
      
      variable_set( 'property_batch_chunk_maximum_limit', 1 );
      
    }
    $limit = ( $limit == 0 ) ? variable_get( 'property_batch_chunk_maximum_limit', 1 ) : $limit; //switch the limit, if we override the chunk limt via cron
    
    if ( is_array( $insertBatch ) ) { //if the insert batch is not empty, run it
      
      foreach ( $insertBatch as $key => $value ) {
        
        if ( $i < $limit ) { //This is not too elegant
          
          self::_shellBatchInsertProperties( $key, $manager );
          $i++;
          
        }
        
      }
      
      unset( $insertBatch );
      
    }
    
    if ( is_array( $updateBatch ) ) { //if the update batch is not empty, run it
      
      foreach ( $updateBatch as $key => $value ) {
        
        if ( $i < $limit ) { //This is not too elegant, and repeated
        
          self::_shellBatchUpdateProperties( $key, $manager );
          $i++;
          
        }
        
      }
      
      unset( $updateBatch );
      
    }
    
    $manager->__destruct();
    
    self::_processMessages(); //Since this is the last phase of the logging
    propertyCache::_clearCache( 'property_suite_transaction_logs', 'cache_property_suite', FALSE );
    
  }
  
  public static function _shellBatchInsertProperties ( $batchNum, $manager ) { //insert from batch, running chunks
        
    $batch = propertyCache::_getCache( 'property_cache_batch_property_insert_' .$batchNum );
    $count = count( $batch );
    
    if ( $batch && $count > 0 ) {
      
      foreach ( $batch as $key => $value ) {
        
        $manager->_insertProperties( $value );
        unset( $batch[$key] ); //This will keep any interruptions from re-batching
          
      }
      
      propertyCache::_clearCache( 'property_cache_batch_property_insert_' .$batchNum, 'cache_property_suite', FALSE ); //when this batch completes, kill it.
      $batchItems = propertyCache::_getCache( 'property_cache_insert_items' );
      unset( $batchItems[$batchNum] );
      propertyCache::_clearCache( 'property_cache_insert_items', 'cache_property_suite', TRUE );
      propertyCache::_setCache( 'property_cache_insert_items', $batchItems );
      
    }
    
    $totalChunks = propertyCache::_getCache( 'property_cache_total_chunks' ) - 1;
    propertyCache::_clearCache( 'property_cache_total_chunks', 'cache_property_suite', TRUE );
    propertyCache::_setCache( 'property_cache_total_chunks', $totalChunks );
    
  }
  
  public static function _shellBatchUpdateProperties ( $batchNum, $manager ) { //update from chunks
    
    $batch = propertyCache::_getCache( 'property_cache_batch_property_update_' .$batchNum );
    $count = count( $batch );
    
    if ( $batch && $count > 0 ) {
      
      foreach ( $batch as $key => $value ) {
        
        $manager->_updateProperties( $value );
        unset( $batch[$key] ); //This will keep any interruptions from re-batching
          
      }
      
      propertyCache::_clearCache( 'property_cache_batch_property_update_' .$batchNum, 'cache_property_suite', FALSE ); //when this batch completes, kill it.
      $batchItems = propertyCache::_getCache( 'property_cache_update_items' );
      unset( $batchItems[$batchNum] );
      propertyCache::_clearCache( 'property_cache_update_items', 'cache_property_suite', TRUE );
      propertyCache::_setCache( 'property_cache_update_items', $batchItems );
      
    }
    
    $totalChunks = propertyCache::_getCache( 'property_cache_total_chunks' ) - 1;
    propertyCache::_clearCache( 'property_cache_total_chunks', 'cache_property_suite', TRUE );
    propertyCache::_setCache( 'property_cache_total_chunks', $totalChunks );
    
  }
  
  /**
   * @name _createShellBatchImages
   * @param object $manager = the manager object you wish to run the _insertImages and _updateImages methods on
   * @return = no return value
   * This function creates a batch for you to initiate from your .sh file. This is meant to be the back end cron run, and requires a re-factoring to make all this work
   */
  public static function _createShellBatchImages ( $manager, $limit = 0 ) {
    
    if ( !ini_get('safe_mode') ) { //Set the ini time, scammed this from devel
      
      set_time_limit(240);
      
    }
    
    $fileTime = propertyCache::_getCache( 'property_cache_filetimes' );
    
    $insertBatch = propertyCache::_getCache( 'property_cache_insert_images' );
    //print_r( $insertBatch );
    $updateBatch = propertyCache::_getCache( 'property_cache_update_images' );
    //print_r( $updateBatch );
    
    $i = 0;
    
    if ( variable_get( 'property_batch_chunk_maximum_limit', null ) == 0 ) { //If for some reason we are getting a zero value, then set it to 1 so at least we can run the update
      
      variable_set( 'property_batch_chunk_maximum_limit', 1 );
      
    }
    $limit = ( $limit == 0 ) ? variable_get( 'property_batch_chunk_maximum_limit', 1 ) : $limit; //switch the limit, if we override the chunk limt via cron
    
    if ( is_array( $insertBatch ) ) { //if the insert batch is not empty, run it
      
      foreach ( $insertBatch as $key => $value ) {
        
        if ( $i < $limit ) { //This is not too elegant
          
          self::_shellBatchInsertImages( $key, $manager, $fileTime );
          $i++;
          
        }
        
      }
      
      unset( $insertBatch );
      
    }
    
    if ( is_array( $updateBatch ) ) { //if the update batch is not empty, run it
      
      foreach ( $updateBatch as $key => $value ) {
        
        if ( $i < $limit ) { //This is not too elegant, and repeated
        
          self::_shellBatchUpdateImages( $key, $manager, $fileTime );
          $i++;
          
        }
        
      }
      
      unset( $updateBatch );
      
    }
    
    $manager->__destruct();
    unset( $fileTime );
    
    self::_processMessages(); //Since this is the last phase of the logging
    propertyCache::_clearCache( 'property_suite_transaction_logs', 'cache_property_suite', FALSE );
    
  }
  
  public static function _shellBatchInsertImages ( $batchNum, $manager, $fileTime ) { //insert from batch, running chunks
  
    $batch = propertyCache::_getCache( 'property_cache_batch_images_insert_' .$batchNum );
    $count = count( $batch );
    
    if ( !empty( $batch ) ) {
      
      foreach ( $batch as $key => $value ) {
        
        $manager->_insertImages( $value, $fileTime );
        unset( $batch[$key] ); //This will keep any interruptions from re-batching
          
      }
      
      propertyCache::_clearCache( 'property_cache_batch_images_insert_' .$batchNum, 'cache_property_suite', FALSE ); //when this batch completes, kill it.
      $batchItems = propertyCache::_getCache( 'property_cache_insert_images' );
      unset( $batchItems[$batchNum] );
      propertyCache::_clearCache( 'property_cache_insert_images' );
      propertyCache::_setCache( 'property_cache_insert_images', $batchItems );
      
    }
    
    $totalChunks = propertyCache::_getCache( 'property_cache_total_image_chunks' ) - 1;
    propertyCache::_clearCache( 'property_cache_total_image_chunks' );
    propertyCache::_setCache( 'property_cache_total_image_chunks', $totalChunks );
    
  }
  
  public static function _shellBatchUpdateImages ( $batchNum, $manager, $fileTime ) { //update from chunks
  
    $batch = propertyCache::_getCache( 'property_cache_batch_images_update_' .$batchNum );
    $count = count( $batch );
    
    if ( !empty( $batch ) ) {
      
      foreach ( $batch as $key => $value ) {
        
        $manager->_updateImages( $value, $fileTime );
        unset( $batch[$key] ); //This will keep any interruptions from re-batching
          
      }
      
      propertyCache::_clearCache( 'property_cache_batch_images_update_' .$batchNum, 'cache_property_suite', FALSE ); //when this batch completes, kill it.
      $batchItems = propertyCache::_getCache( 'property_cache_update_images' );
      unset( $batchItems[$batchNum] );
      propertyCache::_clearCache( 'property_cache_update_images' );
      propertyCache::_setCache( 'property_cache_update_images', $batchItems );
      
    }
    
    $totalChunks = propertyCache::_getCache( 'property_cache_total_image_chunks' ) - 1;
    propertyCache::_clearCache( 'property_cache_total_image_chunks' );
    propertyCache::_setCache( 'property_cache_total_image_chunks', $totalChunks );
    
  }
  
  /**
   * @name _analyzeCache
   * @param string $propertyKey = the represented token you wish to use for the unique property identifyier, such as MLS num, or Property Num
   * @param string $imageID the token that represents the key that the imag uses in the XML
   * @param string $imagePath = the string
   * @param string $dateInserted the token that respresents the date inserted key
   * @param string $dateModified = the token that represents the date updated key
   * @return = no return value
   * This analyzes a cache source and turn the results into meaningful data that the Property Suite Module can parse in a batch or by cron
   */
  public static function _analyzeCache( $propertyKey = '[Mls_Num]', $imageID = 'imagesavailable', $imagePath = null, $dateInserted = '[Date_Inserted]', $dateModified = '[Date_Modified]' ) {
    
    //Clear all cached data
    propertyCache::_clearCache( 'property_cache_batch_property_insert_', 'cache_property_suite', TRUE );
    propertyCache::_clearCache( 'property_cache_batch_property_update_', 'cache_property_suite', TRUE );
    propertyCache::_clearCache( 'property_cache_insert_items', 'cache_property_suite', FALSE );
    propertyCache::_clearCache( 'property_cache_update_items', 'cache_property_suite', FALSE );
    
    $properties         = array(); //This is the array we use to make the chunks
    $newCacheProperties = array(); //This is how we calculate old properties
    $importedProperties = array(); //This is hoe we evaluate if a property is in the system or not
    
    $xml = propertyCache::_getCache( 'property_cache_new_cache' );
    
    $xml = propertyTrans::_validateData( $xml );
    
    if ( $xml ) { //if we have an object, then we want to analyze it's pieces and set cache parts
      
      $importedProperties = self::_getImportedProperties( $propertyKey, $dateModified ); //Get the properties that have already been imported
      
      //MLS Number Token from Admin
      $mlsNumKey = token_replace( $propertyKey, 'property', 'Property Suite' );
      
      //Date Inserted, used to check if images need to be replaced
      $dateInsertedKey = token_replace( $dateInserted, 'property', 'Property Suite' );
      
      //Date Updated, used to check if images need to be replaced
      $dateModifiedKey = token_replace( $dateModified, 'property', 'Property Suite' );
      
      foreach ( $xml->item as $key => $value ) { //convert the xml to a workable array, and chunk it
        
        $mlsNum = (string) $value->$mlsNumKey; //Get the MLS Number
        
        $dateInserted = $value->$dateInsertedKey; //Get the Date Inserted
        
        $dateModified = $value->$dateModifiedKey; //Get the Date Modified
        
        //we have to see right here if the date Modified is greater than the date inserted, so we can ignore properties that have not been updated
        //The only way we can do that is to load the node here, and compare the last dates.
        
        $status = array_key_exists( $mlsNum, $importedProperties ) ? true : false ;
        $set    = ( $status == true ) ? 'update' : 'insert';
        
        if ( $status == false || strtotime( $dateModified ) > strtotime( $importedProperties[$mlsNum]['date_modified'] ) ) { //If the RAM Date is higher than the local Update Time, do your thing
  
          $properties[$set][$mlsNum] = $value; //prepare the property
          
        }        
  
        $newCacheProperties[$mlsNum] = $value; //This is the array we use to see if there are properties that need to be deleted
        
      }
    
      //Set MLS Numbers, so we have a list of current properties to display, from both the insert and update arrays
      $insertMlsNum = ( count( $properties['insert'] ) > 0 ) ? array_keys( $properties['insert'] ) : array(); 
      $updateMlsNum = ( count( $properties['update'] ) > 0 ) ? array_keys( $properties['update'] ) : array();

      //threshold to run batches at, so we don't run into execution problems
      $total = count( $properties['insert'] ) + count( $properties['update'] );
      
      $logs['Cache'][] = array(
        'title'       => t( 'Property' ),
        'description' => t( 'The Property Module parsed !count properties into the system.', array( '!count' => $total ) ),
        'severity'    => REQUIREMENT_INFO,
        'value'       => 'Propreties Parsed',
      );
      
      //Cache the total items
      propertyCache::_setCache( 'property_cache_items_total', $total );
      $maxExecution = ( ini_get('max_execution_time') > 240 ) ? 240 : ini_get('max_execution_time'); //make sure this doesn't blow out things
      $threshold    = 5; //Themagical number of nodes in one batch seems to be 10, so this is our threshold DO NOT CHANGE!!
      
      //chunk
      $insertChunks = ( count( $properties['insert'] ) > 0 ) ? array_chunk( $properties['insert'], $threshold, true ) : array(); //Chunked Insertable properties
      $updateChunks  = ( count( $properties['update'] ) > 0 ) ? array_chunk( $properties['update'], $threshold, true ) : array(); //Chunked Updateable properties
  
      ( count( $insertChunks ) > 0 ) ? propertyCache::_setCache( 'property_cache_insert_items', $insertChunks ) : propertyCache::_clearCache( 'property_cache_insert_items' ); // Cache Chunked Insertable properties
      ( count( $updateChunks ) > 0 ) ? propertyCache::_setCache( 'property_cache_update_items', $updateChunks ) : propertyCache::_clearCache( 'property_cache_update_items' ); // Cache Chunked Updateable properties
      
      //total chunks
      $totalChunks  = count( $insertChunks ) + count( $updateChunks );
      propertyCache::_setCache( 'property_cache_total_chunks', $totalChunks ); //now that we have everything preserved, they can batch out
      
      //batch items, flush, insert batches and update batches
      propertyCache::_clearCache( 'property_cache_batch', 'cache_property_suite', TRUE );
      
      if ( count( $insertChunks ) > 0 && is_array( $insertChunks ) ) {
        
        foreach ( $insertChunks as $key => $value ) { //get all the batches into the cache
          
          propertyCache::_setCache( 'property_cache_batch_property_insert_' .$key, $value );
          
        }
      
      }
      
      if ( count( $updateChunks ) > 0 && is_array( $updateChunks ) ) {
        
        foreach ( $updateChunks as $key => $value ) { //get all the update batches into the cache
          
          propertyCache::_setCache( 'property_cache_batch_property_update_' .$key, $value );
          
        }
      
      }
      
      //cleanup orphaned properties
      self::_processOrphans( $newCacheProperties, $importedProperties );
      unset( $properties );
      unset( $insertChunks );
      unset( $updateChunks );
      unset( $xml );
      
    } else { //if for some reason we do not have valid xml, kill the re-cache
      
      $logs['Cache'][] = array(
        'title'       => t( 'Property' ),
        'description' => t( 'The Property Module could not parse the XML into a valid Object. This is an unrecoverable error, please check the cached XML data, you have to do this manually.' ),
        'severity'    => REQUIREMENT_ERROR,
        'value'       => 'XML Validation',
      );   

    }
    
    self::_logMessages( $logs ); //Log The messages
    self::_processMessages(); //Since this is the last phase of the logging
    propertyCache::_clearCache( 'property_suite_transaction_logs', 'cache_property_suite', FALSE );
    
  }
  
  /**
   * @name _analyzeCacheImages
   * @param string $propertyKey = the represented token you wish to use for the unique property identifyier, such as MLS num, or Property Num
   * @param string $imageID the token that represents the key that the imag uses in the XML
   * @param string $imagePath = the string
   * @param string $dateInserted the token that respresents the date inserted key
   * @param string $dateModified = the token that represents the date updated key
   * @return = no return value
   * This analyzes a cache source and turn the image results into meaningful data that the Property Suite Module can parse in a batch or by cron
   */
  public static function _analyzeCacheImages( $propertyKey = '[Mls_Num]', $imageID = 'imagesavailable', $imagePath = null, $dateInserted = '[Date_Inserted]', $dateModified = '[Date_Modified]' ) {
    
    $newCacheImages = array();
    
    $xml = propertyCache::_getCache( 'property_cache_new_cache' );
    
    $xml = propertyTrans::_validateData( $xml );
    
    if ( $xml ) { //if we have an object, then we want to analyze it's pieces and set cache parts
      
      $importedProperties = self::_getImportedProperties( $propertyKey, $dateModified ); //Get the properties that have already been imported
      
      //MLS Number Token from Admin
      $mlsNumKey = token_replace( $propertyKey, 'property', 'Property Suite' );
      
      //Date Inserted, used to check if images need to be replaced
      $dateInsertedKey = token_replace( $dateInserted, 'property', 'Property Suite' );
      
      //Date Updated, used to check if images need to be replaced
      $dateModifiedKey = token_replace( $dateModified, 'property', 'Property Suite' );
      
      foreach ( $xml->item as $key => $value ) { //convert the xml to a workable array, and chunk it
        
        $mlsNum = (string) $value->$mlsNumKey; //Get the MLS Number
        
        $dateInserted = $value->$dateInsertedKey; //Get the Date Inserted
        
        $dateModified = $value->$dateModifiedKey; //Get the Date Modified
        
        $status = array_key_exists( $mlsNum, $importedProperties ) ? true : false ;
        $set    = ( $status == true ) ? 'update' : 'insert';
        
        if ( $status == false || strtotime( $dateModified ) > strtotime( $importedProperties[$mlsNum]['date_modified'] ) ) { //If the RAM Date is higher than the local Update Time, do your thing
          
          if ( $value->$imageID ) { //@upload images into the property_suite central directory @ the cache stage, so the node_save does not time out when curling images
            
            $images = explode( "-", $value->$imageID );
            
            if ( is_array( $images ) ) { //If we have successfully found images
              
              foreach ( $images as $image ) { //blow out images, and store them
                
                $newCacheImages[$set][$image] = array(
                  'imagePath'    => $imagePath,
                  'image'        => $image,
                  'dateModified' => (string) $dateModified,
                  'mlsNum'       => $mlsNum,
                );
                
              }
              
            } else { //if we have one image use that
              
              $image = trim( $value->$imageID );
              $newCacheImages[$set][$image] = array(
                'imagePath'    => $imagePath,
                'image'        => $image,
                'dateModified' => (string) $dateModified,
                'mlsNum'       => $mlsNum,
              );
              
            }
            
            unset( $images );
            
          }
          
        }        
        
      }

      //threshold to run batches at, so we don't run into execution problems
      $total = count( $newCacheImages['insert'] ) + count( $newCacheImages['update'] );
      propertyCache::_setCache( 'property_cache_images_total', $total );
      
      $logs['Images'][] = array(
        'title'       => t( 'Property' ),
        'description' => t( 'The Property Module parsed !count images into the system.', array( '!count' => $total ) ),
        'severity'    => REQUIREMENT_INFO,
        'value'       => 'Images Parsed',
      );
      
      $threshold = 100; //The amount of images per batch - this is a hard coded number because there can be so many images at one time
      
      //chunk
      $insertChunks = ( count( $newCacheImages['insert'] ) > 0 ) ? array_chunk( $newCacheImages['insert'], $threshold, true ) : array(); //Chunked Insertable properties
      $updateChunks  = ( count( $newCacheImages['update'] ) > 0 ) ? array_chunk( $newCacheImages['update'], $threshold, true ) : array(); //Chunked Updateable properties
  
      ( count( $insertChunks ) > 0 ) ? propertyCache::_setCache( 'property_cache_insert_images', $insertChunks ) : propertyCache::_clearCache( 'property_cache_insert_images' ); // Cache Chunked Insertable properties
      ( count( $updateChunks ) > 0 ) ? propertyCache::_setCache( 'property_cache_update_images', $updateChunks ) : propertyCache::_clearCache( 'property_cache_update_images' ); // Cache Chunked Updateable properties
      
      //total chunks
      $totalChunks  = count( $insertChunks ) + count( $updateChunks );
      propertyCache::_setCache( 'property_cache_total_image_chunks', $totalChunks ); //now that we have everything preserved, they can batch out
      
      //batch items, flush, insert batches and update batches
      propertyCache::_clearCache( 'property_cache_batch_images', 'cache_property_suite', TRUE );
      
      if ( count( $insertChunks ) > 0 && is_array( $insertChunks ) ) {
        
        foreach ( $insertChunks as $key => $value ) { //get all the batches into the cache
          
          propertyCache::_setCache( 'property_cache_batch_images_insert_' .$key, $value );
          
        }
      
      }
      
      if ( count( $updateChunks ) > 0 && is_array( $updateChunks ) ) {
        
        foreach ( $updateChunks as $key => $value ) { //get all the update batches into the cache
          
          propertyCache::_setCache( 'property_cache_batch_images_update_' .$key, $value );
          
        }
      
      }
      
      unset( $newCacheImages );
      unset( $insertChunks );
      unset( $updateChunks );
      unset( $xml );
      
    } else { //if for some reason we do not have valid xml, kill the re-cache
      
      $logs['Images'][] = array(
        'title'       => t( 'Property' ),
        'description' => t( 'The Property Module could not parse the XML into a valid Object. This is an unrecoverable error, please check the cached XML data, you have to do this manually.' ),
        'severity'    => REQUIREMENT_ERROR,
        'value'       => 'XML Validation `  ', 
      );   

    }
    
    //Set filetimes for image analysis - This allows us to compare files
    self::_setFileTimes();
    
    self::_logMessages( $logs ); //Log The messages
    self::_processMessages(); //Since this is the last phase of the logging
    propertyCache::_clearCache( 'property_suite_transaction_logs', 'cache_property_suite', FALSE );
    
  }
  
  /**
   * @name _processOrphans
   * Process All orhpaned nodes now, as you process the new cache
   * @param array $newCacheProperties an array of newly cache properties
   * @param string $propertyKey the token key used to identify the property mls num
   */
  public static function _processOrphans($newCacheProperties, $importedProperties) {
    //Find Orphaned Properties so they can be unpublished or deleted
    $staleness  = variable_get( 'property_property_staleness', '90 days' );
    //Loop through the property rs and see if we need to unpublish any properties
    foreach ( $importedProperties as $key => $property ) {
      
      $status = ( array_key_exists( $key, $newCacheProperties ) ) ? 'publish' : 'unpublish';

      if ( $status == 'unpublish' ) { //Just unpublish right here, unless the node should be deleted
        
        $node       = node_load( $property['nid'] );
        $changed    = date('Y-m-d', $node->changed);
        $expiration = strtotime( "$changed + $staleness" );
        //drupal_set_message(t('Imported Property: !mls - Status: !status | Last Changed !changed, Expiration - !expiration | Node Status: !node_status', array('!mls' => $key, '!status' => $status, '!changed' => $changed, '!expiration' => date('m-d-Y', $expiration), '!node_status' => $node->status )));
        
        if ( $node->status == 1 ) { //If we are not past our expiration date, just unpublish - we want this here, because deleting the node means deleting images, which means if something bad happens, then we don't have to go through re-caching a billion images

          $logs['Orphans'][] = array(
            'title'       => t( 'Property' ),
            'description' => t( 'The Property Module unpublished !title at !link, as it has been identified as not on the current property list.', array( '!link' => l( 'View This Node', 'node/' .$node->nid ), '!title' => $node->title ) ),
            'severity'    => REQUIREMENT_WARNING,
            'value'       => 'Property Unpublished',
          );
          
          //Unpublish the node here - because there can be so many reasons this fails on node_save, manually update the node entry here - this ensures we unpublish without error on malformed properties
          db_query("UPDATE {node} SET status = %d WHERE nid = %d", 0, $node->nid);
          
        } else if ( $expiration <= strtotime( "now" ) && $node->status == 0 )  { //If we are past expiration, delete me
          
          $logs['Orphans'][] = array(
            'title'       => t( 'Property' ),
            'description' => t( 'The Property Module permanenetly deleted !title.', array( '!title' => $node->title  ) ),
            'severity'    => REQUIREMENT_INFO,
            'value'       => 'Property Deleted',
          );
          
          //If content is deleted, this may throw an erro rin the future - I just don't know
          content_delete( $node );
          node_delete( $node->nid );
          
        }
        
        unset( $node );
        
      }
      
    }
    
    unset( $importedProperties );
    unset( $newCacheProperties );
    self::_logMessages( $logs ); //Log The messages
    
  }
  
  /**
   * @name _setFileTimes()
   * Get a master list of files and file times, so we don't have to make many mysql calls at once
   * @return array $fileTime a list of files from the property and their file times, so we can compare new and old images
   * $fileTime[filepath] = timestamp
   */
  public static function _setFileTimes() {
    
    propertyCache::_clearCache( 'property_cache_filetimes', 'cache_property_suite', TRUE );
    
    $fileTime = array();
    
    //Get the count from the Data Base
    $fileCount = db_result( db_query( "SELECT COUNT(filepath) FROM {files} WHERE filepath LIKE '%s%'", PROPERTY_MODULE_UPLOAD_DIRECTORY ) );
    $chunks    = round( $fileCount/1000 );
    unset( $fileCount );
    
    //Set the limit into a number set
    for ( $i = 0; $i < $chunks; $i++ ) {
      
      $limit = $i * 1000;
      
      //Loop through the limit and use this to make out fileTime array
      $rs = db_query( "SELECT timestamp, filepath FROM {files} WHERE filepath LIKE '%s%' LIMIT ".$limit.", 1000", PROPERTY_MODULE_UPLOAD_DIRECTORY ); //Look for the timestamp and turn into a filepath array
      
      while ( $res = db_fetch_array( $rs ) ) {
        
        $path = $res['filepath'];
        $fileTime[$path] = $res['timestamp'];
        
      }
      
      unset( $rs );
      
    }
    
    propertyCache::_setCache( 'property_cache_filetimes', $fileTime );
    
  }
  
  /**
   * @name _getImportedProperties
   * Get the imported properties as defined in the node table
   * @param string $propertyKey the token key used to identify the property ml num
   * @return array $importedProperties[mls_num] = array( nid, date_modified )
   */
  public static function _getImportedProperties( $propertyKey = null, $dateModified = null ) { 
    
    $importedProperties = array();
    
    $rs = self::_propertyRecordset( $propertyKey, $dateModified );
    
    if ( $rs ) {
      
      while ( $res = db_fetch_array( $rs ) ) { //I hate repeating this later, so @todo make property comparison another function
        
        $mlsNum                      = $res['mls_num'];
        $importedProperties[$mlsNum] = array( 'nid' => $res['nid'], 'date_modified' => $res['date_modified'] );
        
      }
      
      unset( $rs );
      
    }
    
    return $importedProperties;
    
  }
  
  /**
   * @name _propertyRecordset
   * Dynamically creates a valid SQL statement to find the property MLS Num from the tokenized mapped value
   * @param string $propertyKey the token key used to identify the property ml num
   * @return object the result of the SQL query
   */
  public static function _propertyRecordset( $propertyKey = null, $dateModified = null ) {
    
    $fieldNames    = variable_get( 'property_content_type_field_names', null );
    //If we don't have a set of field names, return false;
    if (empty($fieldNames)) { return false; }

    //Run the property Type.
    $propertyType  = variable_get( 'property_content_type', null );
    
    foreach ( $fieldNames as $field ) {
      
      $key = "[" .variable_get( $field, null ) ."]";
      
      if ( $key == $propertyKey ) { //MLS Num
        
        $viewFieldPropertyKey = str_replace( 'property_content_type_' .$propertyType .'_', '', $field ); //strip out the extra property_content_type_%%_ so we can get the proper value into the query
        
      }
      
      if ( $key == $dateModified ) { //Date Modified

        $viewFieldPropertyDateModified = str_replace( 'property_content_type_' .$propertyType .'_', '', $field ); //strip out the extra property_content_type_%%_ so we can get the proper value into the query

      }
      
    }
    
    if ( $viewFieldPropertyKey && $viewFieldPropertyDateModified ) {
      
      //generate a valid SQL satement, depending on your particular situation
      $columnMLS      = 'node_data_' .$viewFieldPropertyKey .'.' .$viewFieldPropertyKey .'_value'; //MLS Num
      $columnMODIFIED = 'node_data_' .$viewFieldPropertyKey .'.' .$viewFieldPropertyDateModified .'_value'; //Date Modified
      
      //@todo this SQL statement needs {} on node
      $sql = "SELECT node.nid AS nid,
              $columnMLS AS mls_num,
              $columnMODIFIED AS date_modified
              FROM node node 
              LEFT JOIN content_type_$propertyType node_data_$viewFieldPropertyKey ON node.vid = node_data_$viewFieldPropertyKey.vid
              WHERE node.type in ('$propertyType') ORDER BY node.nid ASC";
      
      return db_query( $sql );
      
    }
    
  }
  
  /**
   * @name _getTotals() - This fixes many problems with cache not reporting back the correct sum of imports, updates and batch results
   * return array $totals an array of the total values returned by a cache
   * $totals['inserted'] = the total inserted items
   * $totals['updated'] = the total updated items
   * $totals['chunks'] = the total chunks processed
   */
  public static function _getTotals() {
    
      
    $totals['total'] = (int) propertyCache::_getCache( 'property_cache_items_total' );
    
    //Total cached items - I hate copying this from ram_property - weak sauce
    if ( is_array( propertyCache::_getCache( 'property_cache_insert_items' ) ) ) { //If the inserted items is an array, then use count()
      
      $totals['insert'] = count( propertyCache::_getCache( 'property_cache_insert_items' ) );
      
    } else {
      
      $totals['insert'] = 0;
      
    }
    
    if ( is_array( propertyCache::_getCache( 'property_cache_update_items' ) ) ) { //If the updated items is an array, then use count()
      
      $totals['update'] = count( propertyCache::_getCache( 'property_cache_update_items' ) );
      
    } else {
      
      $totals['update'] = 0;
      
    }
    
    $totals['chunks']      = $totals['insert'] + $totals['update'];
    $totals['last_import'] = '';
    $totals['imported']    = '';
    
    return $totals;
    
  }
  
  /**
   * @name _getTotalsImages() - This fixes many problems with cache not reporting back the correct sum of imports, updates and batch results
   * return array $totals an array of the total values returned by a cache
   * $totals['inserted'] = the total inserted items
   * $totals['updated'] = the total updated items
   * $totals['chunks'] = the total chunks processed
   */
  public static function _getTotalsImages() {
    
      
    $totals['total'] = (int) propertyCache::_getCache( 'property_cache_images_total' );
    
    //Total cached items - I hate copying this from ram_property - weak sauce
    if ( is_array( propertyCache::_getCache( 'property_cache_insert_images' ) ) ) { //If the inserted items is an array, then use count()
      
      $totals['insert'] = count( propertyCache::_getCache( 'property_cache_insert_images' ) );
      
    } else {
      
      $totals['insert'] = 0;
      
    }
    
    if ( is_array( propertyCache::_getCache( 'property_cache_update_images' ) ) ) { //If the updated items is an array, then use count()
      
      $totals['update'] = count( propertyCache::_getCache( 'property_cache_update_images' ) );
      
    } else {
      
      $totals['update'] = 0;
      
    }
    
    $totals['chunks']      = $totals['insert'] + $totals['update'];
    $totals['last_import'] = '';
    $totals['imported']    = '';
    
    return $totals;
    
  }
  
  /**
   * @name _storeImage() = curls the image to the local server
   * This function evaluates if the images need to be updated, or inserted based on their values
   * @param string $imagePath = the external path to the image
   * @param string $image = the image file we need to parse
   * @param string $dateModified = the string time that we need to compare to the file field datetime, to figure out if it too old and needs to be replaced
   * @param string $mlsNum = the mls nmber for this property
   * @param array $fileTime = an array of all the file times in the system, $fileTime[filepath] = timestamp
   * @return boolean TRUE/FALSE this is just a return value, no real reason to have it
   */
  public static function _storeImage( $imagePath, $image, $dateModified, $mlsNum, $fileTime ) {
    
    if ( !$image && !$imagePath ) { return FALSE; } //If the image path is not valid return false
    
    //We need to query the database for changes in th timestamp, and flag out of date images as replaceable
    $localPath  = PROPERTY_MODULE_UPLOAD_DIRECTORY;
    $localImage = $localPath.$image;
    $replace    = FALSE; //If the file exists, or it is out of date, then update it
    $fileExists = FALSE;

    if ( file_exists( $localImage ) ) { //If the file is already here, then figure out if it needs to be replaced
      
      if ( !empty( $fileTime[$localImage] ) && !empty( $dateModified ) ) { //If we have both a modified date, and the file time is in the list, run the update
            
        //If the update time is less than the timestamp
        $timeStamp = strtotime( trim( $dateModified ) );
  
        $updateMSG = t( ' - Last Remote Update: !lastUpdated | File Timestamp !fileTime ', array( '!lastUpdated' => date( 'Y-m-d', $timeStamp ), '!fileTime' => date( 'Y-m-d', $fileTime[$localImage] + 14400 ) ) );
        
        if ( $fileTime[$localImage] < $timeStamp && $fileTime[$localImage] + 14400 < $timeStamp ) {
          //If the timestamp on the file is less than the $dateModified, and there is a corrsponding record replace -
          //we also have to account for timezone offset, so this also advances the time stamp 4 hours if they are still off, then replace - this should handle a four hour time difference between servers and browser
          
          $fileExists = TRUE;
          $replace    = TRUE;
          
        }
      
      } else { //I want to for sure orce this to not replace
        
        $replace    = FALSE; //If the file exists, or it is out of date, then update it
        $fileExists = FALSE;
        
        $logs['Images'][] = array(
          'title'       => t( 'Property Images' ),
          'description' => t( 'FATAL: COULD NOT FIND A FILETIME OR MODIFIED DATE !path for Property #!mls !updateMSG', array( '!path' => l( 'This Image', $localImage ), '!mls' => $mlsNum, '!updateMSG' => ' - File Insert' ) ),
          'severity'    => REQUIREMENT_WARNING,
          'value'       => 'Update Image',
        );
        
      }
      
    } else { //If the file is not there, then put it in
      
      $fileExists = FALSE;
      $replace    = TRUE;
      
    }
    
    if ( $replace == TRUE ) { //If we need to curl or replace the file, do it - this will also add in new images
      
      $ch = curl_init();
      curl_setopt( $ch, CURLOPT_RETURNTRANSFER, 1 ); //Set curl to return the data instead of printing it to the browser.
      curl_setopt( $ch, CURLOPT_URL, $imagePath.$image ); //Set the URL for the image
      $data = curl_exec( $ch );
      curl_close( $ch );
      
      if ( $data ) {
          
        if ( $fileExists === TRUE ) { //Set a notice if we update and have replaced the file
          
          file_save_data( $data, $localImage, FILE_EXISTS_REPLACE ); //Replace the file
          usleep(1000); //Just in case this is timing out
          
          //update timestamp on the image, so it updates correctly with the properties update time - this is probably
          $rs  = db_query( "UPDATE {files} SET timestamp = %d WHERE filepath LIKE '%s'", $timeStamp, $localImage );
          unset( $rs );
          
          //flush the image cache for this image if we are updating it
          if ( function_exists( 'imagecache_image_flush' ) ) {
            
            imagecache_image_flush( $localImage );
            
          }
          
          //Log Messages
          if ( variable_get( 'property_suite_debug_images', null ) == 'all' || variable_get( 'property_suite_debug_images', null ) == 'notice' ) {
            
            $logs['Images'][] = array(
              'title'       => t( 'Property Images' ),
              'description' => t( 'Updated the image: !path for Property #!mls !updateMSG', array( '!path' => $localImage, '!mls' => $mlsNum, '!updateMSG' => $updateMSG  ) ),
              'severity'    => REQUIREMENT_INFO,
              'value'       => 'Update Image',
            );
            
          }
          
        } else { //Set an Info entry if we insert
          
          //Delete Unwated information in case for some reason we have images with fids
          file_delete( $localImage );
          $rs  = db_query( "DELETE FROM {files} WHERE filepath LIKE '%s'", $localImage );
          unset( $rs );
          
          //Insert New Image
          $filepath = file_save_data( $data, $localImage, FILE_EXISTS_ERROR ); //Replace the file
          usleep(1000); //Just in case this is timing out
          
          //flush the image cache for this image if we are updating it
          if ( function_exists( 'imagecache_image_flush' ) ) {
            
            imagecache_image_flush( $localImage );
            
          }
          
          //Log Messages
          if ( variable_get( 'property_suite_debug_images', null ) == 'all' || variable_get( 'property_suite_debug_images', null ) == 'info' ) {
            
            if ( $filepath !== 0 ) {
              
              $logs['Images'][] = array(
                'title'       => t( 'Property Images' ),
                'description' => t( 'Inserted !path for Property #!mls !updateMSG', array( '!path' => l( 'This Image', $localImage ), '!mls' => $mlsNum, '!updateMSG' => ' - File Insert' ) ),
                'severity'    => REQUIREMENT_OK,
                'value'       => 'Insert Image',
              );
              
            } else {
              
              $logs['Images'][] = array(
                'title'       => t( 'Property Images' ),
                'description' => t( 'COULD NOT Insert the image: !path for Property #!mls !updateMSG', array( '!path' => $localImage, '!mls' => $mlsNum, '!updateMSG' => ' - File Insert' ) ),
                'severity'    => REQUIREMENT_ERROR,
                'value'       => 'Insert Image',
              );
              
            }
          
          }
          
        }
        
        unset( $data );

      } else {
        
        $logs['Images'][] = array(
          'title'       => t( 'Property Images' ),
          'description' => t( 'Could Not CURL The Data: !path for Property #!mls', array( '!path' => $localImage, '!mls' => $mlsNum ) ),
          'severity'    => REQUIREMENT_ERROR,
          'value'       => 'CURL/HTTP Error',
        );
        
      }
      
      self::_logMessages( $logs );
      
    }
    
  }
  
  /**
   * @param object $data = the data you wish to validate
   * @return mixed returns either boolean false or the xml object, so you can check for success
   */
  public static function _validateData( $data = null ) { //Queries the XML source for a valid object
    
    $data = simplexml_load_string( $data );
    
    if ( is_object( $data ) ) { //return the valid data object, this is a prime example of how you can override something through a child class
      
      return $data;
      
    } else { //or return false, so you can add your own success check
      
      return false;
      
    }
    
  }
  
  /**
   * @param string $url = the url you wish to validate
   * @return mixed returns either boolean false or the xml object, so you can check for success
   */
  public static function _validateURL( $url = null ) {
    
    return ( valid_url( $url, true ) ) ? $url : false;
    
  }
  
  /**
   * @param $op = the op you wish to execute: 'test_conection', etc
   * @param object $manager the manager you wish to commit to
   * return array $connect 
   * $conect['data'] = the data object that comes back from the manager class
   * $connect['log'] = a log of the data
   */
  public static function _commit( $op = 'test_connection', $manager ) {
    
    if ( is_string( $op ) ) {
      
      switch ( $op ) {
        
        case 'batchProperties':
          
          $manager->_batchProperties();
          
        break;
      
        case 'batchImages':
          
          $manager->_batchImages();
          
        break;
        
        case 'runtime':
        case 'test_connection':
        
          $connect['data'] = $manager->_connect();
          $connect['log']  = $manager->_shutdown();
        
        break;
        
      }
      
      $manager->__destruct();
      
      return $connect;
      
    } else { //if this is false, you have not constucted the commit properly
      
      return false;
      
    }
    
  }

}