##Property Suite Notes

##Concept

The property suite module is a set of extendible classes and ajax callbacks that one can use to create their own connector or property sub-module. This module on it's own does nothing by itself,
you must create a sub-module ( like RAM, which is supplied, but not activated by default ) that delegates actions to the property API functions.

The concept of this module is to reduce, reuse, recycle, extend and eliminate anything you do not need in your sub-module. This also creates a centralized and standardized way of safely importing property nodes into Drupal without server strain, and automatically adjusts itself to differing server resources.

##File Overview

This module creates a directory /property and property/thumbs in your drupal sites/default/files - this is a system path, and must be maintained. In future releases, I will add support for different files, or connect them to a CCK image field that you define.

##Overview

The workflow is to have an administrator/cron

A) Retrieve the current cache from your data source.
B) Analyze f images need to be imported or updated in the database and physical file location.
C) Analyze if properties need to be inserted or updated.
D) Set the number of batches to be processed.

This module has been optimized to separate arrays and optimize efficiency, using node_load as sparingly as possible. Since this module analyzes a lot of nodes and data, the process uses a lot of memory.

Image uploads are huge in this module, so you need a good chunk of server space. Remember, realty these days is all about the pictures. The average pull for images on 2500 records is over 17000, with a MB total at 600MB's of space. I've seen some sites have 30,000 images+.

The actual cached node data (XML string) for RAM is very low considering, only 4MB of cached data in total for 2500 properties, so other than the images uploaded in the first cache, the import and update process is not so server intensive. Once you have the first cache of images and data, you need to run the manual update again, or just let cron run as the system will play catch-up. Update: Some RAM XMl can be over 15 MB which takes long to upload.

Re-cache, updates and inserts invoke on cron runs, and manual caching/importing is still available. This module favors inserts over updates, so new properties show up quickly, and updates happen second. Since this process is actually pretty stable, you should not notice a difference in inserting vs. updating.

##Server Requirements

Max execution time seems to have an effect on caching images. I am finding on my local machine, that the image caching process takes much longer, and that it does not download all the images only half that on the server in one run. NOTE: In the module, I have changed the execution time with ini_set, and this seems to have allowed all the images to import. However, in general practice, you may notice that importing of all images takes a few caches to run, so when you start try running the cache up to five times, or until the "cache" process takes less than a minute to complete, this is unfortunately the only way to ensure you have received all the images, as most third party services do not report back to you total images, etc.

Memory usage needs to be somewhere in the 128MB-256MB, it is very memory intensive to upload and update nodes.

##Installation

1. Install the Property Suite Module first, because of module weighting issues, this must happen first.
2. Install any subsequent modules AFTER you install the Property Suite Module to retain the proper weighting of the Property Suite API

##CCK Support

As of this writing, Apr 2010 this module currently supports:

CCK Text Widget ( Textfield, Text area, Float, Integer ), single or unlimited values - I have not tested a set number of values in this module, but it should work.

CCK File field ( Image field ), unlimited values - I have not tested this for single values.

CCK support has only been implemented on a limited basis. Fields can use allowed values, however when using allowed values remember that a node may not upload if the allowed value does not exist. Please refer to the example content type export for more information.

Also, the CCK widget link is not supported, you can use a textfield and output that field as a link by using views to show nodes with panels. See Road Map for more information.

To make searches work, you need to expose the allowed values from your content type in the view filters. Then, you can search for whatever you wish.

Lastly, when using the multiple values, set to unlimited if you wish to have more than one value, setting to a static number, such as 6, and you do not have exactly 6 fields to put in the content type, CCK will invalidate that node for import. Selecting unlimited will correct this issue with CCK.

Also, when selecting a field to have a multiple value, the system will automatically take the string, ex: "foo; bar" and translate it to array( 'foo', 'bar' );
If the field has a delimiter of ex: "foo, bar" then the array will be array( 'foo',  'bar' );
If no array delimiter is found, then it defaults to a single value as a string.

##Cron Runs

Cron Runs in two steps:

1) Cache

	i. Cache the current data reported from your third party API
	ii. Analyze the cache, import/update images and assign the data to the "Insert" or "Update" batch process

2) Batch
	
	i. Save New Nodes
	ii. Update Older Nodes
	iii. Un-publish orphaned nodes
	iv. Delete Stale Nodes

##Setup

There is no set up for the Property module in general, however listed below is the setup instructions for the RAM module.

##RAM Setup

Api Setup

This is where you input the API string (the url) in which you are connecting to. Since this is absolutely necessary and unique to each setup, this locks itself after you commit it. The only
way to change this is to reset the module in it's entirety. Yes, it is that specific.

Connection

After you set the API string, you can then test the connection to the server. Since most servers have an IP and domain restriction, when you change from a dev environment to live
you must change this value, and re-connect to the server by testing the connection.

Tokens

This module depends on Drupal's token module. This allows you to maintain your own set of tokens that match the keys that each individual API uses. Having tokens allows this module
to be independent of any modules that call and make their own tokens, so you do not have to re-write this module each time for each different scenario.

Filters

Filters are controlled by a sub-module, like RAM API. Since filtering or setting up queries for individual modules can be daunting, each sub-module must control it's own filters,
which allows for unique property key, image key, image path, broker id, etc. You must set the values you need here.

Cache

After completing the setup steps before this, you may now cache the new data from the connection and query you just created. This can be a lengthy process, and it is recommended that
you in fact run the caching process a few times, because of execution limits on the server. I have benchmarked different caches from different servers, Server A taking one cache run to cache everything from RAM, and Server B taking three different runs to cache all the images. Also, I have noticed on different days that RAM reports different numbers of properties, depending on if Brokers are editing or have updated their property in their MLS interface, such as mlexchange.com. It may take a few caching/batches to import everything from RAM.

Content Type Mapping

SET UP PATH-AUTO ALIASES BEFORE YOU IMPORT!!!

After you create the initial cache, you now have all the information needed by the system to map the content you have already imported, or the content type you wish to make. You must create your content type first, then select it from the list provided by the RAM module, then you assign fields to mapping tokens, and save the configuration. Please refer to CCK Support for more information on supported content fields and their interactions with the Property API.

It is also recommended that you DO NOT MAP TO:

Link (Can't add titles to url's, or affect the link's _target attributes via node_save) : rewrite the field for any url's in views.
Date (This is just a mess to use with node_save. The array is impossible to decipher reliably) You may use this as an extra imported field though, with a default value.
User Reference ( Too many crazy problems to mention ) : No alternative solution available, needs patch
Node Reference ( Inconsistent handling of the node reference array, changes with versions of the content module ) : No alternative solution available, needs patch
Any CCK field not referenced from above.

You can use the afore mentioned fields on their own, but not for data mapping purposes as of Release Candidate 2. As soon as those modules are updated, or remove legacy issues (Such as the Date CCK widget) then this module will support them.

**Notes on Formatting**
If you use Allowed values in your CCK field, then you must use plain text, because filtered text automatically adds markup on the node_save process, which adds invalid markup to your field. Also, if you are using allowed values, you need to make sure the field is set to unlimited, so that the form does not error out.

**Notes on Allowed Values**
I am finding that if you are using allowed values in CCK fields, that you should wait till your initial full import of all nodes to place them. CCK will fail any node_save without error, and is a pain to track down. NOTE: I have a lot of this eliminated, through some pre-checks, but every now and then I run into CCK allowed values validator.

After your initial bulk import, then, you need to create a summary view
showing all values added ( by creating a views argument, and listing it as a summary if no argument is present ), then use that list as your allowed values, so people can search. The module will report any errors as a watchdog log.

It is also best to add keyed arrays with numeric values as keys for CCK allowed values, but honestly that part is up to you, I get varied results with the Allowed values syntax.

**Notes on URL Aliases**

SET UP PATH-AUTO ALIASES BEFORE YOU IMPORT!!!

You must set up url aliases before you begin your import, you don't want to have to mess with url aliases after the fact, because you need to delete the aliases from database, not through
the path-auto api. I made the mistake of erasing my auto-alias before I started the import, and it took some serious trickery to get everything out and the paths deleted, and re-imported.

Import/Update

IF YOU ARE RUNNING YOUR FIRST IMPORT, BACKUP FIRST!!!!!!!

If you do not backup, let the great bird of the galaxy help you. Let me say this once more, for heaven's sake backup before making your first import, or if manually importing. This is just good practice, don't make a NOOB decision. WARNING: This module does not back up for you, so it is best practice to use the backup/migrate module and set up regular backups so you are not hosed.

Lastly, did I mention to backup?!

After you have completed the connection, tokens, filters, caching and Content Mapping you can now import a batch.

It is recommended that on the first batch, you accept the default batch of 1 batch, and let one batch run. If anything is wrong, you can restore from your backup, (you made one right?) and
start the process over, and tweak your content type and start again. This allows you to test your pieces, and get it right before you turn control over to cron.

If you have completed all of this, then you are ready to go and complete your site! You can now use Drupal as you understand it, to make searches, property details, all the good stuff you need! YAY!!

##Property API Map

As of Dec 2010 the Property API is at version RC3. This is a release candidate, and all bugs and feature requests should be sent to paul@bluetent.com Release Candidate 2 addresses performance issues in the RAM API and fixes some cURL issues in the property API and extends the propertyAPI to allow for concatenating multiple tokens in multiple fields.

The Property API as it stands has many Classes that are extendible. The API is broken down into:

propertyManager = This is the base implementation of the API's functions, so that they act as a manager for the delegating functions. This allows all other classes to expand from the main interface without error. Currently this is a blank slate, and as certain functions become "globally" accepted, they will become migrated into this class, which you can invoke in your module by using _parent::_fooFunction() [This is a lot like the views API]

propertyAdmin = This public static class controls all admin functions for a module, such as form generation and various administrative scope functions, anything that needed to be standardized.

propertyCache = This public static class controls all caching through the propertyAPI. If you are caching, use these functions as they are protective to caching data for this API.

propertyTrans = This public static class controls all transactions, transformations, transitions, translations, through the propertyAPI. This is the core of how the property suite runs a submodules $manager object

propertyInstall = At install, the entire module is not available, this makes those classes available to the property installer.

%my-module%Manager = Your module's manager object is how your sub module communicates processes via propertyTrans::_commit to the property API. Your manager object you create, allows you total flexibility within the Property API. You can override any of the default manager objects functions, in fact you must override them, as they are blank and merely placeholders for the API.

And, the best thing is you can ignore it all, use what you want in your module and retain your own modules autonomy, or leech from the API as you wish! Sweet!

##Security

As of RC1 there are no known security issues.

##Road Map (Todo's)

@todo turn the large amount of variables that are system orientated into a serialized array
@todo make the setup process more streamlined: _checkStage _setStage
@todo turn the "manually cache*" separated options into one field-set with radio buttons: Cache Properties | Cache Images | Cache All
@todo turn the "manually import*" separated options into one field-set with radio buttons: Import Properties (Only shows if we import images) | Import Images
@todo allow for user assigned delimiters to translate to array keys for multiple CCK values - not hardcoded, but a variable
@todo add support for the date api and CCK date fields
@todo add help section
