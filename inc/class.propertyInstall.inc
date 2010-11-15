<?php

/**
 * @name propertInstall
 * @category Module Install
 */
class propertyInstall {

  /**
   * @param array $module an array to add to the variable property_suite_modules
   * @param string $module['foo']['name'] = the machine name of the module
   * @param string $module['foo']['title'] = the title you gave the module
   * @param string $module['foo']['value'] = any relevent value you need to give, most likely a version number or other information
   * @param string $module['foo']['description'] = the description you wish to give the module you are loading.
   */
  public static function _registerModule( $module ) {
    
    if ( is_array( $module ) ) {
      
      //register module in the suite
      $modules = variable_get( 'property_suite_modules', array() );
      
      $modules = array_merge( $module, $modules );
      
      variable_set( 'property_suite_modules', $modules );
      
      drupal_set_message( t( "!name has been registered as the master module.", array( '!name' => $module[key($module)]['title'] ) ), 'status' );
      
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
      
      drupal_set_message( t( "!name has been unregistered from the Property Suite module.", array( '!name' => ucwords($module) ) ), 'status' );
      
      menu_rebuild(); //Rebuild menues when a module registers, and un registers
    
    }
    
  }
  
}