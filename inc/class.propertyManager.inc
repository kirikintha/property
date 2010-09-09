<?php
/**
 * class propertyManager
 * @todo - Why is this giving a fatal error when I use implements factoryManager?
 */
class propertyManager {
  
  public function __construct() {}
  
  public function __destruct() { unset( $this ); }
  
  public function _query() {}
  
  public function _connect() {}
  
  public function _batchProperties() {}
  
  public function _updateProperties( $data = null ) {}
  
  public function _insertProperties( $data = null ) {}
  
  public function _shutdown() {}
  
}