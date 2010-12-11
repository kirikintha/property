/**
 * Miscellaneous Functions
 * 
 */
if (Drupal.jsEnabled) {
  
	$(document).ready( function() {
    
    /**
     * Commit Buttons
     * AJAX: Test Connection, Tokenize, Cache
     * This makes for a pretty interface
     */
    $("#edit-test-connection").bind( 'click', function() {
    
      _commit( $(this), null, '_ram_property_test_connection', '#test-progress', 10, 100, 'Starting Test', 'Test completed, just a moment while we update the module configuration...' );
      
      return false;
    
    } );
    
    $("#edit-property-connection-tokenize").bind( 'click', function() {
    
      _commit( $(this), $("#edit-property-tokens").val(), '_ram_property_generate_tokens', '#token-progress', 10, 100, 'Generating Tokens', 'Token Generation Completed' );
      
      return false;
    
    } );
    
    $("#edit-property-cache-all").bind( 'click', function() { //cache all from server
      
      _commit( $(this), null, '_ram_property_retrieve_new_cache', '#cache-progress', 10, 25, 'Retrieving New Cache From Server.', 'New Cache retrieved.' );
      
      _commit( $(this), null, '_ram_property_analyze_new_cache', '#cache-progress', 25, 50, 'Analyzing New Cache.', 'Analysis completed.' );
      
      _commit( $(this), null, '_ram_property_analyze_new_images', '#cache-progress', 50, 75, 'Analyzing New Cached Images.', 'Analysis completed.' );
      
      _commit( $(this), null, '_ram_property_process_orphans', '#cache-progress', 75, 100, 'Processing Orphans.', 'Analysis completed.' );

      return false;
    
    } );

  } );


}