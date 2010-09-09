/**
 * Miscellaneous Functions
 */
if (Drupal.jsEnabled) {
  
  // Make sure our objects are defined for theme objects.
  Drupal.propertySuite = Drupal.propertySuite || {};
  
	$(document).ready( function() {
    
    $('select#edit-property-batch-chunk-maximum-limit, select#edit-ram-property-batch-cron-limit').bind( 'change', function() {
      
      alert('You have changed the maximum number of batches to run, please press: Save Configuration');
      
    });

  } );
  
  /**
   * @param string item = $(this)
   * @param string data = a data string to manipulate
   * @param string callback = the function or static function that you are trying to call ie: _foo_function or foo::fooFunction
   * @param string target = the element that you are targeting to show the progress bar
   * @param integer i = the percentage that starts the $.post
   * @param integer total = the percentage when the $.post completes
   * @param string starMessage = a string as the property processes
   * @param string endMessage = a message to display when the progress reaches 100%
   */
  function _commit( item, data, callback, target, i, total, startMessage, endMessage ) { //commit allows the committal process to run a callback, so that the phases of the commit process can have a progress abr
    
    var j = i; //Allows us to manipulate the set interval
    
    Drupal.theme( 'progress', target, i, total, startMessage, endMessage, false );

    $( ".form-submit").hide(); //hide the other submit buttons
    
    $.ajax( { //This is a synchronous request to and from the server
      
      url: Drupal.settings._property_suite_admin_ajax_commit,
      global: true,
      type: "POST",
      data: ( { callback: callback, data: data } ),
      dataType: "json",
      async: false,
      success: function( msg ) {
        
        Drupal.theme( 'progress', target, 100, total, startMessage, endMessage, false );
        
        if ( total == 100 ) {
          
          $("div.progress div.filled").css( "background", "#DDFFDD");
          $("div.progress div.filled").css( "border-bottom", "0.5em solid #BBEEBB");
          
          setTimeout( function () { window.location.reload(); } , 1500 );
          
        }
        
      },
      error: function( errorMSG ) {
        
        Drupal.theme( 'progress', target, 100, total, startMessage, endMessage, false );
        
        if ( total == 100 ) {
          
          $("div.progress div.filled").css( "background", "#FFCCCC");
          $("div.progress div.filled").css( "border-bottom", "0.5em solid #EEBBBB");
          
          setTimeout( function () { window.location.reload() } , 3500 );
          
        }
        
      }
    
    } );

  }

}