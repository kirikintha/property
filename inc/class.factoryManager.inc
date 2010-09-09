<?php
/**
 * @name propertyManager
 * @category Module Interface.
 */
interface factoryManager {
  
  public function _query();
  
  public function _connect();
  
  public function _batchProperties();
  
  public function _updateProperties( $data = null );
  
  public function _insertProperties( $data = null );
  
  public function _shutdown();
  
}