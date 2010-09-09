#!/usr/local/bin/php
<?php
//#!/usr/bin/php
// ------------------------------------------------------------
// Required customization
// ------------------------------------------------------------
// The name of your site. Required so that when we bootstrap Drupal in
// this script, we find the right settings.php file in your sites folder.
//define('SITE_NAME', 'sre.localhost:8082' );
//define('SITE_NAME', 'sonnenalp.dev2.webenabled.net' ); //WEBENABLED VARIABLE
//define('SITE_NAME', 'sonnenalprealestate.com' ); //LIVE VARIABLE
define('SITE_NAME', 'sre.he243.vps.webenabled.net' ); //NEW WEBENABLED LIVE VARIABLE - TEMP!!!

// The root of your Drupal installation, so we can properly bootstrap
// Drupal. This should be the full path to the directory that holds
// your index.php file, the "includes" subdirectory, etc.
//define('DRUPAL_ROOT', '/Users/Paul/Sites/sandbox.drupal/btmsites.net/sre.btmsites.net/'); //local host directory - COMMENT ME OUT!
//define('DRUPAL_ROOT', '/home/clients/websites/w_sre/public_html/sre/' ); //WEBENABLED VARIABLE
//define('DRUPAL_ROOT', '/var/www/vhosts/sonnenalprealestate.com/httpdocs/'); //Live site directory
define('DRUPAL_ROOT', '/home/clients/websites/w_sre/public_html/sre/' ); //NEW WEBENABLED LIVE VARIABLE

// ------------------------------------------------------------
// Initialization
// (Real work begins here, nothing else to customize)
// ------------------------------------------------------------

// Check if all required variables are defined
$vars = array(
  'DRUPAL_ROOT' => DRUPAL_ROOT,
  'SITE_NAME'   => SITE_NAME
);

$fatal_err = FALSE;
$name      = array();
foreach ( $vars as $key => $val ) {
  
  if ( empty( $val ) ) {
    
    $name[]    = $key;
    $fatal_err = TRUE;
    
  }
  
}

if ( $fatal_err ) {
  
  $name = implode( ", ", $name );
  print "FATAL: \"$name\" constant not defined, aborting cron run from ram_property_cron.php\n";
  
  exit(1);
  
}

//Setup variables for Drupal bootstrap
$script_name = $argv[0];
$_SERVER['HTTP_HOST'] = SITE_NAME;
$_SERVER['REQUEST_URI'] = $_SERVER['SCRIPT_NAME'] = $_SERVER['PHP_SELF'] = '/' . $script_name;
$_SERVER['SCRIPT_FILENAME'] = $_SERVER['PATH_TRANSLATED'] = $_SERVER['PWD'] .'/'. $script_name;

if ( !chdir(DRUPAL_ROOT) ) {
  
  print "FATAL: Can't chdir(" .DRUPAL_ROOT ."), Aborting.\n";
  
  exit(1);
  
}

// Make sure our umask is sane for generating directories and files.
umask(022);

//Invoke Bootstrap
include_once( DRUPAL_ROOT.'/includes/bootstrap.inc' );
drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
$GLOBALS['base_path'] = '/'; //keep the base path correct for l();
$GLOBALS['base_url']  = $GLOBALS['base_root']; //Make sure the bases match up

//Require that Property Module
if ( !module_exists('property') ) {
  
  print "FATAL: Property Module NOT found via shell access, Aborting.\n";
  
  $watchdogMsg = array(
    '!msg' => t( 'Property Module NOT found via shell access, Aborting.' ),
  );
  watchdog( t( 'Property Suite' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_CRITICAL, 'Cron SH' ); //watchdog log
  
  exit(1);
  
}

//Require that RAM Property Module
if ( !module_exists('ram_property') ) {
  
  print "FATAL: RAM Property Module NOT found via shell access, Aborting.\n";
  
  $watchdogMsg = array(
    '!msg' => t( 'RAM Property Module NOT found via shell access, Aborting.' ),
  );
  watchdog( t( 'RAM Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_CRITICAL, 'Cron SH' ); //watchdog log
  
  exit(1);
  
}

//Run Cron if it exists, if it doesn't we know that there is a directory error, or something module side is broken
if ( function_exists( 'ram_property_cron_sh' ) ) {
  
  if ( function_exists('_property_init_invoke' ) ) {
    
    _property_init_invoke(); //guess we have to invoke this on cron @todo make this a class?
    
    if ( function_exists('ram_property_cron_sh' ) ) {
      
      print "SUCCESS: RAM Property Cron SH successfully started from the shell.\n";
      
      $watchdogMsg = array(
        '!msg' => t( 'RAM Property Cron SH successfully started from the shell.' ),
      );
      watchdog( t( 'RAM Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_INFO, 'Cron SH' ); //watchdog log
      
      //Make sure the global variables are in tact
      $watchdogMsg = array(
        '!msg' => print_r( $GLOBALS, TRUE ),
      );
      watchdog( t( 'RAM Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_DEBUG, 'Cron SH - VARIABLES LOG' ); //watchdog log
      
      $totals = propertyTrans::_getTotals(); //Get the totals from the last cache call
      if ( $totals['chunks'] == 0 ) { //Sempahore Sanity Check
        
        variable_set( 'ram_property_cron_run_semaphore', 'recache' );
        
      } else {
        
        variable_set( 'ram_property_cron_run_semaphore', 'batch' );
        
      }
      
      $user = user_load( 1 ); //Load Super User, this is probably not the best, but whatever
      $GLOBALS['user'] = $user;
      ram_property_cron_sh();
      
    } else {
      
      print "FATAL: RAM Property Cron SH could not load from the shell.\n";
      
      $watchdogMsg = array(
        '!msg' => t( 'RAM Property Cron SH could not load from the shell.' ),
      );
      watchdog( t( 'RAM Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_CRITICAL, 'Cron SH' ); //watchdog log
      
      ram_property_cron_sh();
      
    }
    
  } else {
    
    print 'Cannot Invoke the Property Module';
    
    $watchdogMsg = array(
      '!msg' => t( 'Property Module could not invoke via shell access, Aborting.' ),
    );
    watchdog( t( 'Property Suite' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_CRITICAL, 'RAM SH' ); //watchdog log
    
  }
  
} else {
  
  print "FATAL: RAM Property Cron cannot find ram_property_cron_sh, Aborting.\n";
  
  $watchdogMsg = array(
    '!msg' => t( 'RAM Property Cron cannot find ram_property_cron_sh, Aborting.' ),
  );
  watchdog( t( 'RAM Property' ), t( '!msg', $watchdogMsg ), array(), WATCHDOG_CRITICAL, 'Cron SH' ); //watchdog log
  
  exit(1);
  
}