#!/usr/bin/python
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
			
			pdb.gimp_image_flatten( image )
			pdb.gimp_file_save( image, image.active_drawable, newFilePath, newFilePath, run_mode=RUN_NONINTERACTIVE )
			pdb.gimp_image_delete( image )

pdb.gimp_quit( TRUE )
