<?php

/**
 * @name propertyCache
 * @category Module Cache.
 */
class propertyCache { //Handles all caching of data in public static functions
  
  /**
   * @param array $caches = a keyed array of $cache objects, as defined below
   * @param object $cache = an object holding the various parameters for the _cacheSet function
   * @param string $cache->cid = the name of the cache table you are referencing
   * @param mixed $cache->data = the data your are serializing
   * @param string $cache->table = the cache table you re referencing
   */
  public static function _rebuildCaches( $caches ) { //rebuild all the cached data in the $caches array
    
    foreach ( $caches as $cache ) {
      
      propertyCache::_setCache( $cache->cid, $cache->data, $cache->expire, $cache->table );
    
    }
    
  }
  
  /**
   * @name _setCache
   * @param string $cid = the name of the cache table you are referencing
   * @param mixed $data = the data your are serializing
   * @param mixed $expire = the expiration time for the cache, default to CACHE_PERMANENT
   * @param string $table = the cache table you're referencing
   * @return false or true, depending on if there is data to cache or not, and if there is a $cid attached
   */
  public static function _setCache( $cid = null, $data = null, $expire = CACHE_PERMANENT, $table = 'cache' ) {
    
    if ( $cid != null && $data != null ) { //serializes the data into the cached table of your choice
      
      propertyCache::_clearCache( $cache, $type, FALSE );
      
      cache_set( $cid, serialize( $data ), $table, $expire ); //everything is permanent in here
      
      return true;
    
    } else {
      
      return false;
      
    }
    
  }
  
  /**
   * @param array $caches = a keyed array of $cache objects, as defined below
   * @param object $cache = an object holding the various parameters for the _cacheSet function
   * @param string $cache->cid = the name of the cache table you are referencing
   * @param mixed $cache->data = the data your are serializing
   * @param boolean $cache->wildcard = true or false, if you are referencing a wildcard parameter for clearing the cache
   */
  public static function _flushCaches( $caches ) { //Kill all the caches in the $caches array
    
    foreach ( $caches as $cache ) {
      
      propertyCache::_clearCache( $cache->cid, $cache->data, $cache->wildcard );
      
    }
    
  }
  
  /**
   * @name _clearCache
   * @param string $cid = the name of the cache table you are referencing
   * @param string $table = the cache table you re referencing
   * @param boolean $wildcard = true or false, if you are referencing a wildcard parameter for clearing the cache
   * @return no return value
   */
  public static function _clearCache( $cid, $table = 'cache', $wildcard = TRUE ) { //clears cached data, defaults to wildcard TRUE
    
    cache_clear_all( $cid, $table, $wildcard );
    
  }
  
  /**
   * @name _getCache
   * @param string $cid = the name of the cache table you are referencing
   * @param string $table = the cache table you re referencing
   * @return mixed $data->data = the unserialized entity you cached
   */
  public static function _getCache( $cid, $table = 'cache' ) { //unserializes the data you serialized, or returns false if no data is present
    
    $data = cache_get( $cid, $table );
    return ( is_object( $data ) ) ? unserialize( str_replace(array('O:16:"SimpleXMLElement":0:{}', 'O:16:"SimpleXMLElement":'), array('s:0:"";', 'O:8:"stdClass":'), $data->data ) ) : false;
    
  }
  
}