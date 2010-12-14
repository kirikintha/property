<?php
/**
 * Property Module Admin
 */

function _property_suite_admin_main() { //Main Admin settings
    
  //System Messages - @the cron can be deprecatd, unecessary
  $form['admin_main'] = array(
    '#type' 		=> 'fieldset',
    '#title' 		=> t('Property Suite Status'),
    '#collapsible' 	=> TRUE,
    '#collapsed' 	=> FALSE,
    '#weight'       => -50,
    '#group'        => 'admin_main'
  );
  
  $form['admin_main']['header'] = array(
    '#value'  => propertyAdmin::_getStatus(),
    '#weight' => -49,
    '#group'  => 'admin_main'
  );
  
  //Map API Keys
  $form['map_api_keys'] = array(
    '#type' 		=> 'fieldset',
    '#title' 		=> t('Map API Keys'),
    '#description'  => t( 'You Must enter Map API keys here for Geocoding purposes. You must also enter one Google API key and One Yahoo API Key, so there is a fallback, in case the system hits API limits.' ),
    '#collapsible' 	=> TRUE,
    '#collapsed' 	=> FALSE,
    '#weight'		=> -49,
    '#group'        => 'map_api_keys'
	);
  
  $form['map_api_keys']['property_suite_map_key_google_geocoder'] = array(
    '#type'           => 'textfield',
    '#title'          => t( 'Google Map API Key' ),
    '#description'    => t( 'Enter your API key here from the google maps API. This will auto-populate if you have already added an API key from the GMAP module.' ),
    '#default_value'  => variable_get( 'property_suite_map_key_google_geocoder', gmap_get_key() ),
    '#required'       => TRUE,
    '#weight'         => -50,
    '#group'          => 'map_api_keys'
  );
  
  $form['map_api_keys']['property_suite_map_key_yahoo_geocoder'] = array(
    '#type'           => 'textfield',
    '#title'          => t( 'Yahoo API Key' ),
    '#description'    => t( 'Enter your Yahoo API geocoding Application ID here.' ),
    '#default_value'  => variable_get( 'property_suite_map_key_yahoo_geocoder', null ),
    '#required'       => TRUE,
    '#weight'         => -49,
    '#group'          => 'map_api_keys'
  );
  
  //Debug options
  $form['debug'] = array(
    '#type' 		=> 'fieldset',
    '#title' 		=> t('Debug Options'),
    '#collapsible' 	=> TRUE,
    '#collapsed' 	=> FALSE,
    '#weight'		=> -48,
    '#group'        => 'debug'
	);
  
  $opts = array(
    0        => t( 'Turn Off Image Debugging. (No Logging)' ),
    'info'   => t( 'Turn On Image Debugging for inserted images only. (Watchdog Info)' ),
    'notice' => t( 'Turn On Image Debugging for updated images only. (Wacthdog Notice)' ),
    'all'    => t( 'Turn On Image Debugging for all image processes only. (Both Info and Notice)' )
  );
  $form['debug']['property_suite_debug_images'] = array(
    '#type'           => 'radios',
    '#title'          => t( 'Turn On Verbose Messages For Images' ),
    '#description'    => t( 'When debugging Image problems, you can turn this option on to log all image transactions via watchdog. NOTE: All warnings and errors are automatically logged into the system. Turn this on for notices and information purposes only. WARNING: This is big!' ),
    '#options'        => $opts,
    '#default_value'  => variable_get( 'property_suite_debug_images', 0 ),
    '#required'       => FALSE,
    '#weight'         => -48,
    '#group'          => 'debug'
  );
  
  $form['#pre_render'][] = 'vertical_tabs_form_pre_render'; //enabled vertical tabs if the module is installed
  
  return system_settings_form( $form );
  
}

function _property_suite_admin_ajax_commit() { //This runs the catch all callbacjk for all ajax
  
  drupal_set_header('Content-Type: text/javascript; charset=utf-8');
  
  header("Expires: Sun, 19 Nov 1978 05:00:00 GMT");
  header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
  header("Cache-Control: no-store, no-cache, must-revalidate");
  header("Cache-Control: post-check=0, pre-check=0", false);
  header("Pragma: no-cache");
  
  $callback = urldecode( $_POST['callback'] );
  $data     = $callback( urldecode( $_POST['data'] ) );
  
  print drupal_to_js( array( 'data' => $data ) );
  
  exit(); //This is the only place it should be uncommented, unles you are debugging a process
  
}

function _property_tokens_validate($form_id, &$form_state) {
  //Remove anything not a pipe or an alphanumeric value.
  $form_state['values']['property_tokens'] = preg_replace("/[\r\n]+[\s\t]*[\r\n]+/","\n", $form_state['values']['property_tokens']);
}