<?php
/**
 * Batch Insert and Update Properties
 * @category Batch API
 */

function _property_suite_admin_batch_properties_finished ( $success, $results, $operations ) { //@todo better messaging
  
  $message = '';

  if ($success) {
    
    $message = 'Property Import Completed.';
    $status  = 'status';
    
    variable_set( 'property_firstrun_imported', true ); //This is our last setup step!
    
    $watchdogMsg = array(
      '!msg' => t( 'Property Module Completed the batch process successfully.' ),
    );
    
    watchdog( t( 'Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_INFO, 'Batch' ); //watchdog log
  
  } else {
    
    $error_operation = reset( $operations );
    $message         = 'An error occurred while processing '. $error_operation[0] .' with arguments :'. print_r( $error_operation, TRUE );
    $status          = 'error';
    
    $watchdogMsg = array(
      '!msg' => t( 'Property Module could not finish the batch process, !message', array( '!message' => $message ) ),
    );
    
    watchdog( t( 'Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_ERROR, 'Batch' ); //watchdog log
  
  }

  drupal_set_message( t( "!message", array ( '!message' => $message ) ), $status );
  
  propertyTrans::_processMessages(); //Since this is the last phase of the logging
  propertyCache::_clearCache( 'property_suite_transaction_logs', 'cache_property_suite', FALSE );
    
}

function _property_suite_admin_batch_insert_properties ( $batchNum, $manager, &$context ) { //insert from batch, running chunks
  
  if ( empty( $context['sandbox'] ) ) {
    
    $context['sandbox']['progress'] = 0;
    $context['finished']            = 0;
  
  }
  
  $context['sandbox']['max'] = variable_get( 'property_batch_chunk_maximum_limit', 1 ); //Total chunk limit.
  $context['sandbox']['progress']++; //This always advances on each batch
  
  $batch = propertyCache::_getCache( 'property_cache_batch_property_insert_' .$batchNum );
  $count = count( $batch );
  
  if ( !empty( $batch ) ) {
    
    foreach ( $batch as $key => $value ) {
      
      $manager->_insertProperties( $value );
      unset( $batch[$key] ); //This will keep any interruptions from re-batching
      
      $context['message'] = t('Processed Insert Batch %batch : %count nodes', array( '%batch' => $batchNum, '%count' => $count ) );
        
    }
    
    propertyCache::_clearCache( 'property_cache_batch_property_insert_' .$batchNum, 'cache_property_suite', FALSE ); //when this batch completes, kill it.
    $batchItems = propertyCache::_getCache( 'property_cache_insert_items' );
    unset( $batchItems[$batchNum] );
    propertyCache::_clearCache( 'property_cache_insert_items' );
    propertyCache::_setCache( 'property_cache_insert_items', $batchItems );
    
  }
  
  $totalChunks = propertyCache::_getCache( 'property_cache_total_chunks' ) - 1;
  propertyCache::_clearCache( 'property_cache_total_chunks' );
  propertyCache::_setCache( 'property_cache_total_chunks', $totalChunks );
  
  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  
}

function _property_suite_admin_batch_update_properties ( $batchNum, $manager, &$context ) { //update from chunks
  
  if ( empty( $context['sandbox'] ) ) {
    
    $context['sandbox']['progress'] = 0;
    $context['finished']            = 0;
  
  }
  
  $context['sandbox']['max'] = variable_get( 'property_batch_chunk_maximum_limit', 1 ); //Total chunk limit.
  $context['sandbox']['progress']++; //This always advances on each batch
  
  $batch = propertyCache::_getCache( 'property_cache_batch_property_update_' .$batchNum );
  $count = count( $batch );
  
  if ( !empty( $batch ) ) {
    
    foreach ( $batch as $key => $value ) {
      
      $manager->_updateProperties( $value );
      unset( $batch[$key] ); //This will keep any interruptions from re-batching
      $context['message'] = t('Processed Update Batch %batch : %count nodes', array( '%batch' => $batchNum, '%count' => $count ) );
        
    }
    
    propertyCache::_clearCache( 'property_cache_batch_property_update_' .$batchNum, 'cache_property_suite', FALSE ); //when this batch completes, kill it.
    $batchItems = propertyCache::_getCache( 'property_cache_update_items' );
    unset( $batchItems[$batchNum] );
    propertyCache::_clearCache( 'property_cache_update_items' );
    propertyCache::_setCache( 'property_cache_update_items', $batchItems );
    
  }
  
  $totalChunks = propertyCache::_getCache( 'property_cache_total_chunks' ) - 1;
  propertyCache::_clearCache( 'property_cache_total_chunks' );
  propertyCache::_setCache( 'property_cache_total_chunks', $totalChunks );
  
  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  
}

/**
 * Batch Insert and Update Images
 * @category Batch API
 */

function _property_suite_admin_batch_images_finished ( $success, $results, $operations ) { //@todo better messaging
  
  $message = '';

  if ($success) {
    
    $message = 'Image Batch Completed.';
    $status  = 'status';
    
    $watchdogMsg = array(
      '!msg' => t( 'Property Module Completed the batch process successfully.' ),
    );
    
    watchdog( t( 'Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_INFO, 'Batch' ); //watchdog log
  
  } else {
    
    $error_operation = reset( $operations );
    $message         = 'An error occurred while processing '. $error_operation[0] .' with arguments :'. print_r( $error_operation, TRUE );
    $status          = 'error';
    
    $watchdogMsg = array(
      '!msg' => t( 'Property Module could not finish the batch process, !message', array( '!message' => $message ) ),
    );
    
    watchdog( t( 'Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_ERROR, 'Batch' ); //watchdog log
  
  }

  drupal_set_message( t( "!message", array ( '!message' => $message ) ), $status );
  
  propertyTrans::_processMessages(); //Since this is the last phase of the logging
  propertyCache::_clearCache( 'property_suite_transaction_logs', 'cache_property_suite', FALSE );
    
}

function _property_suite_admin_batch_insert_images ( $batchNum, $manager, $fileTime, &$context ) { //insert from batch, running chunks
  
  if ( empty( $context['sandbox'] ) ) {
    
    $context['sandbox']['progress'] = 0;
    $context['finished']            = 0;
  
  }
  
  $context['sandbox']['max'] = variable_get( 'property_batch_chunk_maximum_limit_images', 1 ); //Total chunk limit.
  $context['sandbox']['progress']++; //This always advances on each batch
  
  $batch = propertyCache::_getCache( 'property_cache_batch_images_insert_' .$batchNum );
  $count = count( $batch );
  
  if ( !empty( $batch ) ) {
    
    foreach ( $batch as $key => $value ) {
      
      $manager->_insertImages( $value, $fileTime );
      unset( $batch[$key] ); //This will keep any interruptions from re-batching
      
      $context['message'] = t('Processed Insert Batch %batch : %count images', array( '%batch' => $batchNum, '%count' => $count ) );
        
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
  
  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  
}

function _property_suite_admin_batch_update_images ( $batchNum, $manager, $fileTime, &$context ) { //update from chunks
  
  if ( empty( $context['sandbox'] ) ) {
    
    $context['sandbox']['progress'] = 0;
    $context['finished']            = 0;
  
  }
  
  $context['sandbox']['max'] = variable_get( 'property_batch_chunk_maximum_limit_images', 1 ); //Total chunk limit.
  $context['sandbox']['progress']++; //This always advances on each batch
  
  $batch = propertyCache::_getCache( 'property_cache_batch_images_update_' .$batchNum );
  $count = count( $batch );
  
  if ( !empty( $batch ) ) {
    
    foreach ( $batch as $key => $value ) {
      
      $manager->_updateImages( $value, $fileTime );
      unset( $batch[$key] ); //This will keep any interruptions from re-batching
      $context['message'] = t('Processed Update Batch %batch : %count images', array( '%batch' => $batchNum, '%count' => $count ) );
        
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
  
  $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['max'];
  
}