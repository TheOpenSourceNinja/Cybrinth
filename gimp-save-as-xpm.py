#!/usr/bin/python
#This script is intended not to be run manually but by make as part of Cybrinth's build process.
import os
imageDir = "images"
compiledImageDir = "compiled-images"
for roots, dirs, files in os.walk( imageDir ):
	for name in files:
		filePath = os.path.join( roots, name )
		if( ( filePath.lower().rfind( ".png" ) != -1 ) or ( filePath.lower().rfind( ".xcf" ) != -1 ) ):
			image = pdb.gimp_file_load( filePath, filePath )
			
			newFilePath = filePath.replace( ".png", ".xpm" )
			newFilePath = newFilePath.replace( ".xcf", ".xpm" )
			newFilePath = newFilePath.replace( imageDir, compiledImageDir )
			
			newDirPath = os.path.dirname( newFilePath )
			if( not os.path.exists( newDirPath ) ):
				os.makedirs( newDirPath )
			
			#pdb.gimp_image_flatten( image )
			pdb.gimp_image_merge_visible_layers( image, CLIP_TO_IMAGE ) #This line is a workaround for a rare GIMP crashing bug. Saving to a format like XPM which does not support layers should cause GIMP to automatically merge all layers, which it does... 99.9999999% of the time.
			#pdb.gimp_file_save( image, image.active_drawable, newFilePath, newFilePath, run_mode=RUN_INTERACTIVE )
			alphaThreshold = 127 #XPM format doesn't do alpha transparency; it does "invisible is a color that pixels can be"
			pdb.file_xpm_save( image, image.active_drawable, newFilePath, newFilePath, alphaThreshold, run_mode=RUN_NONINTERACTIVE )
			pdb.gimp_image_delete( image )

pdb.gimp_quit( TRUE )
