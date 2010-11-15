<?php
/**
 * class propertyManager
 */
class propertyManager {

  protected $url;
  protected $ch;
  public $qry;
  public $batchProcessPath;
  
  public function __construct() {}
  
  public function __destruct() { unset( $this ); }
  
  public function _query() {}
  
  public function _connect() {}
  
  public function _batchProperties() {}
  
  public function _updateProperties( $data = null ) {}
  
  public function _insertProperties( $data = null ) {}
  
  public function _shutdown() {}
  
}