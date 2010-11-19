<?php
/**
 * @name ramManager
 * @category module manager
 */
class ramManager extends propertyManager {
  
  public function __construct() {
    
    $this->url = propertyTrans::_validateURL(variable_get('ram_property_connection_string', NULL));
    $this->batchProcessPath = 'admin/settings/property_suite/ram_property';
    
  }

  public function __destruct() {
    unset($this);
  }
  
  /**
   * @name _connect
   * @param string $postData an XML request from the RAM server
   * @return mixed returns either boolean FALSE or the xml object, so you can check for successful query objects
   */
  public function _connect() {
    
    $query['input'] = $this->qry;
    
    $this->ch  = curl_init($this->url);
    //Set our CURL options to ensure a proper transfer.
    curl_setopt($this->ch, CURLOPT_CONNECTTIMEOUT, 30); //If the connection times out in 30 seconds, then bail out.
    curl_setopt($this->ch, CURLOPT_TIMEOUT, 60); //If the response is longer than 60 seconds, bail.
    curl_setopt($this->ch, CURLOPT_RETURNTRANSFER, 1); //Set the curl data to return data.
    curl_setopt($this->ch, CURLOPT_POST, 1); //Set this to post the data to RAM.
    curl_setopt($this->ch, CURLOPT_POSTFIELDS, $query); //Set the fields that we are posting, this is the query we are applying to RAM.
    //Run our curl request.
    $data = curl_exec($this->ch);
    //Return Data.
    return $data;
    
  }
  
  /**
   * @name _shutdown
   * @return html a list of log values
   */
  public function _shutdown() {
    
    $log[] = t('CURL Info: !time', array('!time' => date("M/d/Y - h:m:s a", strtotime("now"))));
    
    $info = (!curl_errno($this->ch)) ? curl_getinfo($this->ch) : curl_error($this->ch) ;
    
    while (list($label, $value) = each($info)) {
      
      $msg = array(
        '!label' => $label,
        '!value' => $value
     );
      
      $log[] = t('!label : !value', $msg);
    
    }
    
    curl_close($this->ch);
    
    return implode("<br />", $log);
    
  }
  
  /**
   * @name _batchProperties
   * @category Module Batch
   * creates a batch to run to update properties
   */
  public function _batchProperties() { //we'll run the batch functions through here, run percent of insert and update here
    
    $manager = new ramManager; //Can I just use self here?
    propertyTrans::_createPropertyBatch($manager); //Don't have to have the limit here, it will run through property
    
  }
  
  /**
   * @name _batchImages
   * @category Module Batch
   * creates a batch to run to update images
   */
  public function _batchImages() { //we'll run the batch functions through here, run percent of insert and update here
    
    $manager = new ramManager; //Can I just use self here?
    propertyTrans::_createImageBatch($manager); //Don't have to have the limit here, it will run through property
    
  }
  
  /**
   * @name _insertImages
   * @category Module Batch
   * Is called as a method of the batch process, as batch runs, the insert is invoked
   */
  public function _insertImages($data = NULL, $fileTime) {
    
    if (is_array($data)) {
      
      propertyTrans::_storeImage($data['imagePath'], $data['image'], $data['dateModified'], $data['mlsNum'], $fileTime);
      
    }

  }
  
  /**
   * @name _updateImages
   * @category Module Batch
   * Is called as a method of the batch process, as batch runs, the update is invoked
   */
  public function _updateImages($data = NULL, $fileTime) {
    
    if (is_array($data)) {
            
      propertyTrans::_storeImage($data['imagePath'], $data['image'], $data['dateModified'], $data['mlsNum'], $fileTime);
      
    }

  }
  
  /**
   * @name _insertProperties
   * @category Module Batch
   * Is called as a method of the batch process, as batch runs, the insert is invoked
   */
  public function _insertProperties($data = NULL) {
    
    $data = (array) $data; //convert object to array, easier to play with this way

    //Get our pieces from the common parser.
    $pieces = self::_getPropertyPieces($data);
    //Get all our pieces we need to run our node update.
    $nid              = $pieces['nid'];
    $fieldNames       = $pieces['fieldNames'];
    $contentFields    = $pieces['contentFields'];
    $agent            = $pieces['agent'];
    $propertyKeyValue = $pieces['propertyKeyValue'];
    $title            = $pieces['title'];

    //If we do not have a title, kill the import.
    if (!empty($title)) {
      // 0 should always be the node title, prepare the node object, and add the title here.
      $node = propertyTrans::_nodePrepare(trim (ucwords(strtolower (urldecode($title)))));
      //Kill off the Node Title, we do not need it now.
      unset($fieldNames[0]);
      //Run through the rest of the fields for this node object and apply the proper values to the proper pieces of the node object.
      foreach ($fieldNames as $field) {
        //Repace token with proper key value, which is key%
        //If we have a location, then we need to process the value differenltly, so multiple tokens are accepted.
        if (strstr($field, "location_street") || strstr($field, "location_additional")) {
          $value = self::_tokensValues($data, variable_get($field, NULL));
        } else {
          //After sanitizing, then make into a real token value.
          $key = "[" .variable_get($field, NULL) ."]";
          $key = (string) token_replace($key, 'property', 'Property Suite');
          //Get the data using the key value.
          $value = (string) $data[$key];
        }
        //Sanitize the value.
        $value = trim (urldecode($value));
        $value = preg_replace('/[^(\x20-\x7F)\x0A]*/', '', $value);
        //After we get the value, convert our field value into a string that corresponds to the node object.
        $field = str_replace('property_content_type_' .variable_get('property_content_type', NULL) .'_', '', $field);
        //If it is a location field, it does not follow CCK API.
        if (strstr($field, "location")) {
          
          if (!empty($value)) {
            
            $location[$field] = trim($value);
          
          }
          
        } else if (strstr($key, variable_get('ram_property_image_key', 'imagesavailable'))) { //If the ley matches the image field map
          
          $imageField = $field;
          
        } else { // else it's a cck field, and we use a diffrerent parser
          
          if (!empty($value)) { //If there is a value, use it - images should ignore this, there is a lot more we can do with CCK in the future, this is really simple eval right now
            
            //if the field has either a ; or a , then convert it to an array, if this cck field has anything more than 1 set as the number of values
            $cck = $contentFields[$field];
            
            if ($cck['multiple'] == 1) { //If the cck field is multiple, break up the values by either ; or ,
              
              $value = (strstr($value, ";")) ? explode(";", $value) : explode(",", $value) ;
              
            }
            
            if (is_array($value) && count($value) > 0) { //If there is a value and it is not empty, then do it
              
              foreach ($value as $multiple => $allowed) {
                  
                array_push($node->$field, array('value' => t(trim($allowed))));
                
              }
              
            } else {
                
              array_push($node->$field, array('value' => t(trim($value)))); //make sure this is a string when it becomes a node
              
            }
  
          }
          
        }
        
      }
      
      if ($location) { //If we have a location, set it
        
        $node = propertyTrans::_nodeLocation($node, $location);
        
      }
               
      //Prepare Images for Insertion into the node
      $imageKEY = variable_get('ram_property_image_key', 'imagesavailable');
      $images   = explode("-", $data[$imageKEY]);

      if (is_array($images) && !empty($images)) { //If there are images in this array, run em because they have already been cached!
        
        foreach ($images as $image) { //blow out images, and store them
          
          $node = propertyTrans::_nodeImage($node, $imageField, $image);
                      
        }
        
      }
          
      //save node, with agent ID
      if (is_object($node)) {
        
        propertyTrans::_nodeSave($node, $agent, $propertyKeyValue, 'Insert'); //@todo add an op to this, so it messages correctly
        
      }   
      
    }

  }

  /**
   * @name _updateProperties
   * @category Module Batch
   * Is called as a method of the batch process, as batch runs, the update is invoked
   */
  public function _updateProperties($data = NULL) {
 
    $data = (array) $data; //convert object to array, easier to play with this way.
    //Get our pieces from the common parser.
    $pieces = self::_getPropertyPieces($data);
    //Get all our pieces we need to run our node update.
    $nid              = $pieces['nid'];
    $fieldNames       = $pieces['fieldNames'];
    $contentFields    = $pieces['contentFields'];
    $agent            = $pieces['agent'];
    $propertyKeyValue = $pieces['propertyKeyValue'];
    $title            = $pieces['title'];

    //Update is just subtly different from insert. @todo - try and get this a little more uniform.
    if (is_numeric($nid) && $nid > 0) { //Double check to make sure the nid is valid
      $node = node_load($nid);
    }
    
    if (!empty($title) && is_object($node)) { //If we do not have a title and the node is not an object, kill the import
      
      $node->title = $title;
      
      /**
       * Unset Unwanted Fields that do not have a token associated with them - this will keep any un-tokened fields from running through the next loop
       */
      unset($fieldNames[0]); //Kill off the Node Title
      
      foreach ($fieldNames as $key => $field) {
        
        $token = variable_get($field, NULL);
        
        if ($token == 'NULL') { //If we have a NULL value, then we don't want to have the field, just unset it
          
          unset($fieldNames[$key]);
          
        }
        
      }
      
      foreach ($fieldNames as $field) {
        
        //If we have a location, then we need to process the value differenltly, so multiple tokens are accepted.
        if (strstr($field, "location_street") || strstr($field, "location_additional")) {
          $value = self::_tokensValues($data, variable_get($field, NULL));
        } else {
          //After sanitizing, then make into a real token value.
          $key = "[" .variable_get($field, NULL) ."]";
          $key = (string) token_replace($key, 'property', 'Property Suite');
          //Get the data using the key value.
          $value = (string) $data[$key];
        }
        //Sanitize the value.
        $value = trim (urldecode($value));
        $value = preg_replace('/[^(\x20-\x7F)\x0A]*/', '', $value);
        //After we get the value, convert our field value into a string that corresponds to the node object.
        $field = str_replace('property_content_type_' .variable_get('property_content_type', NULL) .'_', '', $field);
        //If it is a location field, it does not follow CCK API.
        if (strstr($field, "location")) { //if it is a location field, put into the node save differently
          
          if (!empty($value)) {
            
            $location[$field] = trim($value);
          
          }
          
        } else if (strstr($key, variable_get('ram_property_image_key', 'imagesavailable'))) { //If the key matches the image field map, set the image field
          
          $imageField = $field;
          
        } else { // else it's a cck field, and we use a diffrerent parser
          
          if (!empty($value)) { //If there is a value, use it - images should ignore this, there is a lot more we can do with CCK in the future, this is really simple eval right now
            
            //if the field has either a ; or a , then convert it to an array, if this cck field has anything more than 1 set as the number of values
            $cck = $contentFields[$field];
            
            if ($cck['multiple'] == 1) { //If the cck field is multiple, break up the values by either ; or ,
              
              $value = (strstr($value, ";")) ? explode(";", $value) : explode(",", $value) ;
              
            }
            
            if (is_array($value)) { //If there is a value and it is not empty, then do it
              
              $node->$field = array(); //clear the array
              
              foreach ($value as $multiple => $allowed) {
                  
                array_push($node->$field, array('value' => t(trim($allowed )))); //unset the original value for this field, then push it to the end
                
              }
              
            } else { //If it is not multiple
                
              $node->$field = array();
              array_push($node->$field, array('value' => t(trim($value)))); //make sure this is a string when it becomes a node
              
            }
  
          }
          
        }
        
      }
      
      if ($location) { //If we have a location, set it
        
        unset($node->locations);
        $node = propertyTrans::_nodeLocation($node, $location);
        
      }
    
      //Prepare Images for Insertion into the node
      $imageKEY = variable_get('ram_property_image_key', 'imagesavailable');
      $images   = explode("-", $data[$imageKEY]);
      if (is_array($images) && !empty($images)) { //We have to compare the images, so they re-order properly.
        
        //Remove Orphaned Images here first
        if (is_array($node->$imageField)) { //We have to take the node field to get the proper name from the CCK field
          
          foreach ($node->$imageField as $key => $image) { //get the images from the image field, and run the update
  
            if (!in_array($image['filename'], $images)) { //If we are not in the array, get rid of this image!
              
              field_file_delete($image, TRUE); //Delete the file from the system entirely, force this!
              
              //It's so simple, just make the image have no FID to delete, since this is a $variable, we have to tranlsate
              $parents = $node->$imageField;
              $parents[$key]['fid'] = NULL;
              $node->$imageField = $parents;
                            
            }
            
          }

        }
        
        foreach ($images as $image) { //blow out images, and store them
          
          $node = propertyTrans::_nodeImage($node, $imageField, $image);
         
        }
        
      }
      
      //save node, with agent ID, and evaluate if it needs to be updated, unpublished, or deleted
      if (is_object($node)) {

        $node->status = 1; //publish me and save, since we may have been unpublished
        
        propertyTrans::_nodeSave($node, $agent, $propertyKeyValue, 'Update');
        
      }
      
    } else { //Destroy the bad node, if it is not good?
      
      
    }

  }

  /**
   * @name _tokensValues()
   * Converts token values from the data source, into a value string to use in node fields.
   * @param array $data = the data array you are passing to the function
   * @param string $tokens = the token(s) string you are passing to the function
   * @return string $value = the converted value(s) we to use in a particular node field.
   */
  public static function _tokensValues($data = NULL, $tokens = NULL) {
    //Turn the values of the tokens back into their key% xml form.
    $keys = token_replace($tokens, 'property', 'Property Suite');
    foreach ($data as $key => $value) {
      //change out the values for the tokens with the values of the keys, this means any token can be used here from the property list
      $values = str_replace($key, urldecode($value), $keys);
    }
    return $values;
  }

  /**
   * @name _getPropertyPieces()
   * Assemble the pieces of the array to return to use in your functions.
   * @return array $pieces = the array of pieces to use in your insert/update schema.
   *  $pieces['nid']
   *  $pieces['fieldNames']
   *  $pieces['contentFields']
   *  $pieces['agent']
   *  $pieces['propertyKeyValue']
   *  $pieces['title']
   */
  public static function _getPropertyPieces($data = NULL) {
    //Set the pieces array.
    $pieces = array();

    //Get node ID from imported nids and create node
    $importedProperties = propertyTrans::_getImportedProperties(variable_get('ram_property_property_key', '[Mls_Num]'), variable_get('ram_property_date_inserted', '[Date_Inserted]'), variable_get('ram_property_date_modified', '[Date_Modified]')); //Get the properties that have already been imported
    $pieces['nid']      = $importedProperties[$data[$propertyKey]]['nid'];

    //Get the fields that have been mapped from the proeprty API.
    $pieces['fieldNames']    = variable_get('property_content_type_field_names', NULL);
    //Get all the fields for this content type.
    $pieces['contentFields'] = content_fields(NULL, variable_get('property_content_type', NULL)); //This is kinda unreliable, but I don't have another option

    //get agent user id @ todo make this into one function for both update and insert for all this stuff
    $agentToken      = token_replace(variable_get('ram_property_broker_key', '[Agent_ID]'), 'property', 'Property Suite');
    $agentID         = $data[$agentToken]; //Agent token is the proper field to query from the data
    $agent           = NULL;
    $pieces['agent'] = propertyTrans::_loadBroker($agentID);//take the broker id, and make the author of this node the, returns a valid user object

    $propertyKey                = (string) token_replace(variable_get('ram_property_property_key', '[Mls_Num]'), 'property', 'Property Suite'); //Why do I need this?
    $pieces['propertyKeyValue'] = $data[$propertyKey]; //This is the value that needs to get passed to the _nodeSave, so we can identify it later for re-cache

    //$fieldNames[0] = the node title.
    //Get the values for each token returned with tokensValues and use that string as the title.
    $title = self::_tokensValues($data, variable_get($fieldNames[0], NULL));
    //Explicitely sanitize the title string.
    $title = preg_replace('/[^(\x20-\x7F)\x0A]*/', '', $title); //strip out non-ascii
    $pieces['title'] = str_replace('&', 'And', $title); //Unforntunately, you can't have ampersands in node titles, ;(

    return $pieces;
  }
  
}

/**
 * @name ramCache
 * @name module cache
 */
class ramCache extends propertyCache {
  
  /**
   * @name _retrieveNewCache()
   * return boolean TRUE FALSE new cache created
   */
  public static function _retrieveNewCache() { //commit that xml query to the $manager

    //Create our RAM query.
    $query = array(
      'numresults' => variable_get('ram_property_query_maximum_limit_default', RAM_PROPERTY_QUERY_MAXIMUM_LIMIT_DEFAULT),
      'offset'     => 0,
      'orderby'    => variable_get('ram_property_orderby', 'Area ASC'),
    );
    //Create our manager, and parse the query into valid RAM XML.
    $manager = new ramManager;
    $manager->qry = ramTrans::_query($query);
    //Set our newCache information.
    $newCache = propertyTrans::_commit('runtime', $manager);

    //If for some reason this bails out, then return FALSE
    if (!$newCache) {
      
      $watchdogMsg = array(
        '!msg' => t('RAM Property Module could not update the cache with new data from the RAM API Server.'),
     );
      watchdog(t('RAM Property'), t('!msg', $watchdogMsg), array(), WATCHDOG_CRITICAL, 'Cache'); //watchdog log
        
      return FALSE;
    
    }

    //If we have a new Cache, then we record the new cache.
    variable_set('ram_property_connect_log', $newCache['log']);

    $watchdogMsg = array(
      '!msg' => t('RAM Property Module sucessfully updated the cache with new data from the RAM API Server.'),
    );
    watchdog(t('RAM Property'), t('!msg', $watchdogMsg), array(), WATCHDOG_INFO, 'Cache'); //watchdog log

    propertyCache::_setCache('property_cache_new_cache', $newCache['data']);

    return TRUE;
    
  }
  
  /**
   * @name _analyzeNewCache()
   * return boolean TRUE FALSE analyze new cache
   */
  public static function _analyzeNewCache() { //retool the xml object into a new array, using the last insert, update, modified data to see what is out of range
      
    propertyTrans::_analyzeCache(variable_get('ram_property_property_key', '[Mls_Num]'), variable_get('ram_property_image_key', 'imagesavailable'), variable_get('ram_property_image_path', NULL), variable_get('ram_property_date_inserted', '[Date_Inserted]'), variable_get('ram_property_date_modified', '[Date_Modified]'));
      
    if (variable_get('ram_property_setup', NULL) == 'cache') {
      
      variable_set('ram_property_setup', 'content');
      variable_set('ram_property_connection_cached', TRUE);
      
    }
    
    return TRUE;
    
  }
  
  /**
   * @name _analyzeNewCacheImages()
   * return boolean TRUE FALSE analyze new cache
   */
  public static function _analyzeNewCacheImages() { //retool the xml object into a new array, using the last insert, update, modified data to see what is out of range
      
    propertyTrans::_analyzeCacheImages(variable_get('ram_property_property_key', '[Mls_Num]'), variable_get('ram_property_image_key', 'imagesavailable'), variable_get('ram_property_image_path', NULL), variable_get('ram_property_date_inserted', '[Date_Inserted]'), variable_get('ram_property_date_modified', '[Date_Modified]'));
    
    return TRUE;
    
  }
  
}

/**
 * @name ramTans
 * @category module transactions
 */
class ramTrans extends propertyTrans {
  
  /**
   * @param array $query = a compiled array that is translated into a search function that RAM accepts
   * @return xml returns a valid XML object
   */
  public static function _query($query = array()) {
    
    $header = array(
      'website'   => variable_get('ram_property_website', NULL),
      'ipaddress' => variable_get('ram_property_ipaddress', NULL),
    );
    
    $queries = array_merge($header, $query);
    
    $xml = new DOMDocument();
    $xml->formatOutput = TRUE;
    
    $rss = $xml->createElement('rss');
    $rss->setAttribute('version', '2.0');
    $xml->appendChild($rss); 
    
    $search = $xml->createElement("search");
    $xml->appendChild($search);
    
    foreach ($queries as $key => $value) {
      
      $key = $xml->createElement((string) token_replace($key, 'property', 'Property Suite')); //replace token values with original values
      $key->appendChild(
        $xml->createTextNode($value)
     );
      
      $search->appendChild($key);
      
    }
    
    return $xml->saveXML();
    
  }
  
}

/**
 * @name ramAdmin
 * @category module administration
 */
class ramAdmin extends propertyAdmin {
  
  /**
   * @name _runCron, evaluates the cron semaphore and runs the appropriate cron stage, re-cache or batch - this should remain specific to each module, so nothing steps on itself cron wise
   * @todo make the semaphore an API piece, that the property suite can have a listener too
   */
  public static function _runCron() {
    
    date_default_timezone_set('America/Denver'); //Because RAM Runs in the Denver MST Time Zone, this has to get set here to America/Denver
    $semaphore   = variable_get('ram_property_cron_run_semaphore', 'recache'); //What stage of cron are we running?
    $currentTime = date("H:00", strtotime("now")); //find out the current time
    $cronTime    = variable_get('ram_property_batch_cron_start_time', '00:00');
    $timeExclude = variable_get('ram_property_batch_cron_time_of_day_exclude', array());
    $insertBatch = propertyCache::_getCache('property_cache_insert_items');
    $updateBatch = propertyCache::_getCache('property_cache_update_items');
    $insertBatchImages = propertyCache::_getCache('property_cache_insert_images');
    $updateBatchImages = propertyCache::_getCache('property_cache_update_images');
    
    if (empty($insertBatch) && empty($updateBatch)) { //Sanity check for the semaphore
      
      variable_set('ram_property_cron_run_semaphore', 'recache');
      
    }
    
    //If we are not in disallowed times, && the start time is less than or equal to now
    if (!in_array($currentTime, $timeExclude) && $cronTime <= date("H:i", strtotime("now"))) {
      
      if ($semaphore == 'recache') { //If we are re-caching, make sure it doesn't fall in disallowed times, and the time is later than the start time, run it and change the semaphore
        
        //print "Retrieving Cache\n";
        ramCache::_retrieveNewCache();
        //print "Cache Received\nAnalyzing Cache\n";
        ramCache::_analyzeNewCache();
        //print "Cache Analyzed, Exiting\n";
        ramCache::_analyzeNewCacheImages();
        //print "Images Analyzed, Exiting\n";
        
        variable_set('ram_property_cron_run_semaphore', 'batch');
        
      } else if (($semaphore == 'batch' && !empty($insertBatchImages)) || ($semaphore == 'batch' && !empty($updateBatchImages))) { //else if we are batching images...
        
        $manager = new ramManager;
        propertyTrans::_createShellBatchImages($manager, variable_get('ram_property_batch_cron_limit', 10));
        
        watchdog(t('RAM Property'), t('Batching Images'), array(), WATCHDOG_INFO, 'Cron Run'); //watchdog log
        
      } else if (($semaphore == 'batch' && !empty($insertBatch)) || ($semaphore == 'batch' && !empty($updateBatch))) { //else if we are batching properties...
        
        $manager = new ramManager;
        propertyTrans::_createShellBatchProperties($manager, variable_get('ram_property_batch_cron_limit', 10));
        
        watchdog(t('RAM Property'), t('Batching Properties'), array(), WATCHDOG_INFO, 'Cron Run'); //watchdog log
        
      } else if ($semaphore == 'batch' && empty($insertBatch) && empty($updateBatch)) { //else if there are no batches to be inserted, set to re-cache
        
        variable_set('ram_property_cron_run_semaphore', 'recache');
        
      }
      
      $watchdogMsg = array(
        '!msg' => t('The RAM Property Module ran the !sempahore cron stage at !runtime. You may check the logs for any additional information.', array('!sempahore' => $semaphore, '!runtime' => date("H:i", strtotime("now")))),
     );
      
      watchdog(t('RAM Property'), t('!msg', $watchdogMsg), array(), WATCHDOG_INFO, 'Cron Run'); //watchdog log
      
      return TRUE;
      
    } else { //Make a watchdog log, just someone can debug slow updates
      
      $watchdogMsg = array(
        '!msg'         => t('The RAM Property Module could not run cron because of time restrictions entered by this module.'),
        '!startTime'   => $cronTime,
        '!currentTime' => $currentTime,
     );
      
      watchdog(t('RAM Property'), t('!msg - Start Time !startTime : Current Time !currentTime', $watchdogMsg), array(), WATCHDOG_INFO, 'Cron Run'); //watchdog log
      
      return FALSE;
      
    }
    
  }
 
  /**
   * @return array $requirements an array of requirements for the site.
   */
  public static function _requirements() {
    
    $requirements = array();
    
    //First Step, this is the connection string, without thism everything else breaks
    $connection = variable_get('ram_property_connection_string', NULL);
    $severity   = (empty($connection) || $connection == FALSE) ? WATCHDOG_CRITICAL : WATCHDOG_OK;
    $value      = (empty($connection) || $connection == FALSE) ? 'No Connection String Established.' : variable_get('ram_property_connection_string', NULL);
    $descr      = (empty($connection) || $connection == FALSE) ? 'Please enter the host name and API file you wish to use for this RAM module. After you enter in a valid connection string, the system will automatically continue to the next step.' : NULL;
    
    $requirements['connection']['connection_string'] = array(
      'title'       => t('API Connection String'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );
    
    //Connection Test
    $tested   = variable_get('ram_property_connection_tested', NULL);
    $severity = (empty($tested) || $tested == FALSE) ? WATCHDOG_CRITICAL : WATCHDOG_OK;
    $value    = (empty($tested) || $tested == FALSE) ? 'Connection NOT established.' : 'Connection established.';
    $descr    = (empty($tested) || $tested == FALSE) ? NULL : NULL;
    
    $requirements['connection']['test_connection'] = array(
      'title'       => t('Server Connection Test'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );
    
    //Tokens
    $tokenize = variable_get('property_connection_tokenized', NULL);
    $severity = (empty($tokenize) || $tokenize == FALSE) ? WATCHDOG_ALERT : WATCHDOG_OK;
    $value    = (empty($tokenize) || $tokenize == FALSE) ? 'Tokens Not Generated.' : 'Tokens Generated.';
    $descr    = (empty($tokenize) || $tokenize == FALSE) ? 'Please press generate tokens to re-cache the tokens. You will then be able to map an association to a content type. Regenterating tokens resets this section, so you will have to update your content type after this.' : NULL;
    
    $requirements['setup']['tokens'] = array(
      'title'       => t('Tokens'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );
    
    //filters
    $filters  = variable_get('ram_property_connection_filtered', NULL);
    $severity = (empty($filters) || $filters != 'complete') ? WATCHDOG_ALERT : WATCHDOG_OK;
    $value    = (empty($filters) || $filters != 'complete') ? 'Filters not created.' : 'Filters Created.';
    $descr    = (empty($filters) || $filters != 'complete') ? 'Please add a set of area filters (at the very least) so you can filter ther results by area.' : NULL;
    
    $requirements['setup']['filters'] = array(
      'title'       => t('Filters'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );
    
    //Cache
    $cached  = variable_get('ram_property_connection_cached', NULL);
    $severity = (empty($cached) || $cached != 'complete') ? WATCHDOG_ALERT : WATCHDOG_OK;
    $value    = (empty($cached) || $cached != 'complete') ? 'First Cache Not Completed' : 'First Cache Compeleted.';
    $descr    = (empty($cached) || $cached != 'complete') ? 'Please set the first cache for the site, since this is most likely a large import, we want to run this manually the first time so we can see how long and how many records are produced.' : NULL;
    
    $requirements['setup']['runtime_cache'] = array(
      'title'       => t('First Run: Cache All'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );
    
    //content types

    //If we have no content type selected, then flag that as an error
    $content  = variable_get( 'property_content_type', NULL );
    $severity = (empty($content)) ? WATCHDOG_ALERT : WATCHDOG_OK;
    $value    = (empty($content)) ? 'No Content Type Selected' : 'Content Type Selected';
    $descr    = (empty($content)) ? 'Please select a content type to use from the list provided, and press save configuration, then continue mapping your content type.' : NULL;

    $requirements['setup']['content_type'] = array(
      'title'       => t('Property Content Type'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
    );

    //If we have a property content type, and we need to map flag this oprion.
    $content  = variable_get('ram_property_setup', NULL);
    $severity = (empty($content) || $content != 'complete') ? WATCHDOG_ALERT : WATCHDOG_OK;
    $value    = (empty($content) || $content != 'complete') ? 'Content Mapping Not Completed' : 'Content Mapping Completed.';
    $descr    = (empty($content) || $content != 'complete') ? 'Please select a content type to import, and then complete the mapping process by assigning tokens to the fields and pressing save configuration.' : NULL;

    $requirements['setup']['content_mapping'] = array(
      'title'       => t('Mapping'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );

    //first run - import
    $firstrun   = variable_get('property_firstrun_imported', NULL);
    $severity = (empty($firstrun) || $firstrun == FALSE) ? WATCHDOG_ALERT : WATCHDOG_OK;
    $value    = (empty($firstrun) || $firstrun == FALSE) ? 'First Batch Import Not Completed' : 'First Batch Import Compeleted.';
    $descr    = (empty($firstrun) || $firstrun == FALSE) ? 'Please import the first batch and make adjustments to your import. You can adjust the number of batches you wish to run (Default: 1 batch), This will complete the setup process.' : NULL;
    
    $requirements['setup']['runtime_import'] = array(
      'title'       => t('First Run: First Import Batch'),
      'value'       => t($value),
      'description' => t($descr),
      'severity'    => $severity
   );
    
    //runtime
    $totals = propertyTrans::_getTotals(); //Get the totals from the last cache call
    if ($totals['chunks'] > 0) {
      
      $msg = array(
        '!total' => $totals['total'],
        '!count' => $totals['chunks']
     );
      
      $requirements['runtime']['items'] = array(
        'title'       => t('Total Items'),
        'value'       => t('!total records, in !count chunks.', $msg),
        'severity'    => REQUIREMENT_INFO
     );
      
      $msg = array(
        '!count' => $totals['insert']
     );
      
      $requirements['runtime']['inserts'] = array(
        'title'       => t('Total Items to insert'),
        'value'       => t('!count batches to insert.', $msg),
        'severity'    => REQUIREMENT_INFO
     );
      
      $msg = array(
        '!count' => $totals['update']
     );
      
      $requirements['runtime']['updates'] = array(
        'title'       => t('Total Items to update'),
        'value'       => t('!count batches to update.', $msg),
        'severity'    => REQUIREMENT_INFO
     );
      
    } else {
      
      $requirements['runtime']['no_cache'] = array(
        'title'       => t('No Cache Created.'),
        'value'       => t('Sorry, there is no cache created, please recreate the cache. This may take a few moments to complete. NOTE: If All Property batches have completed, you will also see this message.'),
        'severity'    => REQUIREMENT_WARNING
     );
      
      variable_set('ram_property_cron_run_semaphore', 'recache');
      
    }
    
    //Image Totals
    $totals = propertyTrans::_getTotalsImages(); //Get the totals from the last cache call
    //krumo($totals);
    if ($totals['chunks'] > 0) {
      
      $msg = array(
        '!total' => $totals['total'],
        '!count' => $totals['chunks']
     );
      
      $requirements['runtime']['images'] = array(
        'title'       => t('Total Images'),
        'value'       => t('!total images, in !count chunks.', $msg),
        'severity'    => REQUIREMENT_INFO
     );
      
      $msg = array(
        '!count' => $totals['insert']
     );
      
      $requirements['runtime']['image_inserts'] = array(
        'title'       => t('Total Images to insert'),
        'value'       => t('!count batches to insert.', $msg),
        'severity'    => REQUIREMENT_INFO
     );
      
      $msg = array(
        '!count' => $totals['update']
     );
      
      $requirements['runtime']['image_updates'] = array(
        'title'       => t('Total Images to update'),
        'value'       => t('!count batches to update.', $msg),
        'severity'    => REQUIREMENT_INFO
     );
      
    } else {
      
      $requirements['runtime']['image_no_cache'] = array(
        'title'       => t('No Image Caches Created.'),
        'value'       => t('Sorry, there is no image cache created, please recreate the cache. This may take a few moments to complete. NOTE: If All Image batches have completed, you will also see this message.'),
        'severity'    => REQUIREMENT_WARNING
     );
      
    }

    return $requirements; 
    
  }
  
}