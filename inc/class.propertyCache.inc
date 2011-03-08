<?php

/**
 * @name propertyCache
 * @category Module Cache.
 * Handles all caching of data in public static functions.
 */
class propertyCache {
  
  /**
   * @param array $caches = a keyed array of $cache objects, as defined below
   * @param object $cache = an object holding the various parameters for the _cacheSet function
   * @param string $cache->cid = the name of the cache table you are referencing
   * @param mixed $cache->data = the data your are serializing
   * @param string $cache->table = the cache table you re referencing
   */
  public static function _rebuildCaches($caches) {
    //Rebuild all the cached data in the $caches array.
    foreach ($caches as $cache) {
      propertyCache::_setCache($cache->cid, $cache->data, $cache->expire, $cache->table);
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
  public static function _setCache($cid = null, $data = null, $expire = CACHE_PERMANENT, $table = 'cache_property_suite') {
    if ($cid != null && $data != null) {
      //Serializes the data into the cached table of your choice.
      propertyCache::_clearCache($cache, $type, FALSE);
      //Everything is permanent in here.
      cache_set($cid, serialize($data), $table, $expire);
      return true;
    } else {
      //$MSG = t('<p>!cid</p><p>!data</p>', array('!cid' => $cid, '!data' => print_r($data, TRUE)));
      //drupal_set_message(t('Cannot set Cache, because either there is no Cache ID or Data. !debug',array('!debug' => $MSG)));
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
  public static function _flushCaches($caches) {
    //Kill all the caches in the $caches array.
    foreach ($caches as $cache) {
      propertyCache::_clearCache($cache->cid, $cache->data, $cache->wildcard);
    }
  }
  
  /**
   * @name _clearCache
   * @param string $cid = the name of the cache table you are referencing
   * @param string $table = the cache table you re referencing
   * @param boolean $wildcard = true or false, if you are referencing a wildcard parameter for clearing the cache
   * @return no return value
   */
  public static function _clearCache($cid, $table = 'cache_property_suite', $wildcard = TRUE) {
    //Clears cached data, defaults to wildcard TRUE.
    cache_clear_all($cid, $table, $wildcard);
  }
  
  /**
   * @name _getCache
   * @param string $cid = the name of the cache table you are referencing
   * @param string $table = the cache table you re referencing
   * @return mixed $data->data = the unserialized entity you cached
   */
  public static function _getCache($cid, $table = 'cache_property_suite') {
    //Unserializes the data you serialized, or returns false if no data is present.
    $data = cache_get($cid, $table);
    return (is_object($data)) ? unserialize(str_replace(array('O:16:"SimpleXMLElement":0:{}', 'O:16:"SimpleXMLElement":'), array('s:0:"";', 'O:8:"stdClass":'), $data->data)) : false;
  }

  /**
   * @name _clearCacheHistory();
   * Clear the property_cache_cache_%date rows in the cache_property_suite table.
   * This function grabs ALL the property_cache_cache_%date rows you request and deletes them.
   * NOTE: This function will permantently erase data!
   * If you are using the wildcard '*' character, this will flush ALL date ranges.
   * If you supply an array to $caches, then it will apply each cache to the _flushCaches method.
   * If you use the default, "HISTORY", this will delete all entries, except for yesterdays Cache.
   */
  public static function _clearCacheHistory($caches = 'HISTORY') {
    if ($caches) {
      //If we have a wildcard, clear all.
      if ($caches == '*') {
        self::_clearCache('property_cache_cache_', 'cache_property_suite', '*');
      } else if (is_array($caches)) {
        //If we have a range, then we take the $range and _flushCaches.
        self::_flushCaches($caches);
      } else if ($caches == 'HISTORY') {
        //Clear all history, except for $today and $yesterday.
        $today     = date('m_d_Y', strtotime('now'));
        $yesterday = date('m_d_Y', strtotime('now - 1 day'));
        $query     = "DELETE FROM {cache_property_suite} WHERE cid LIKE 'property_cache_cache_%' AND NOT LIKE 'property_cache_cache_%_bak' AND cid NOT IN ('property_cache_cache_$today', 'property_cache_cache_$yesterday')";
        db_query($query);
      }
    }
  }
  
}