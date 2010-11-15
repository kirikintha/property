/**
 * Miscellaneous Functions
 */
if (Drupal.jsEnabled) {
  
  // Make sure our objects are defined for theme objects.
  Drupal.propertySuite       = Drupal.propertySuite || {};
  Drupal.propertySuite.theme = Drupal.propertySuite.theme || {};
  
  $(document).ready( function() {
    
    $('select#edit-property-batch-chunk-maximum-limit, select#edit-ram-property-batch-cron-limit').bind( 'change', function() {
      alert('You have changed the maximum number of batches to run, please press: Save Configuration');
    });

  });

  //End of document.ready()
  
  /**
   * @name _commit()
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
    
    Drupal.theme( 'propertySuiteAdminProgress', target, i, total, startMessage, endMessage, false );

    $( ".form-submit").hide(); //hide the other submit buttons
    
    $.ajax( { //This is a synchronous request to and from the server
      
      url: Drupal.settings._property_suite_admin_ajax_commit,
      global: true,
      type: "POST",
      data: ( { callback: callback, data: data } ),
      dataType: "json",
      async: false,
      success: function( msg ) {
        
        Drupal.theme( 'propertySuiteAdminProgress', target, 100, total, startMessage, endMessage, false );
        
        if ( total == 100 ) {
          
          $("div.progress div.filled").css( "background", "#DDFFDD");
          $("div.progress div.filled").css( "border-bottom", "0.5em solid #BBEEBB");
          
          setTimeout( function () { window.location.reload(); } , 1500 );
          
        }
        
      },
      error: function( errorMSG ) {
        
        Drupal.theme( 'propertySuiteAdminProgress', target, 100, total, startMessage, endMessage, false );
        
        if ( total == 100 ) {
          
          $("div.progress div.filled").css( "background", "#FFCCCC");
          $("div.progress div.filled").css( "border-bottom", "0.5em solid #EEBBBB");
          
          setTimeout( function () { window.location.reload() } , 3500 );
          
        }
        
      }
    
    } );

  }

  /**
   * JS Theme Elements
   */

  /**
   * @name = Drupal.theme.prototype.propertySuiteAdminProgress
   * Emulates the progress bar from drupal, without having to use AHAH, and you can use your own whatever to make this happen
   * Useage: Drupal.theme('progress', target, i, total, startMessage, endMessage, false);
   * @param string target = the name of the div you are targeting, you may use classes and ids, target anything you wish
   * @param integer i = the initial value to make a percentage from
   * @param integer total = the total, which calculates the percentage against i
   * @param string startMessage = the message to display when the percentage is less than 100%
   * @param string endMessage = the message to display when the percentage is 100%
   */
	Drupal.theme.prototype.propertySuiteAdminProgress = function (target, i, total, startMessage, endMessage, fade) {
      var percent;
      if (fade == null || fade == undefined) { fade = false; }
      if (i >= 0 && total >= i) {
        percent = Math.round((parseInt(i) + 1) / parseInt(total) * 100);
        $("#theme-prototype-progress").remove();
        if (percent < 100) {
          $(target).prepend(Drupal.propertySuite.theme.progressBar(percent, startMessage));
        } else if (percent >= 100) {
          percent = 100;
          $(target).prepend(Drupal.propertySuite.theme.progressBar(percent, endMessage));
          if (fade != false) { //If fade is enabled
            setTimeout(function() { $("#theme-prototype-progress").fadeOut('slow'); } , 2000);
          }
        }
      }
      return false;
	}

  /**
   * @name Drupal.PHTools.theme.progressBar
   * Progress Bar - html wrapper for Drupal.theme('progress');
   */
	Drupal.propertySuite.theme.progressBar = function (percent, message) {
      var output;
      if (percent && message) {
        output = '<div id="theme-prototype-progress" class="progress">';
        output += '<div class="bar"><div class="filled" style="width: '+ percent +'%"></div></div>';
        output += '<div class="percentage">'+ percent +'%</div>';
        output += '<div class="message">'+ Drupal.t(message) +'</div>';
        output += '</div>';
        return output;
      } else {
        output = 'No Progress Bar can be made, you need a percentage and a message, please try again';
        return false;
      }
	}

}