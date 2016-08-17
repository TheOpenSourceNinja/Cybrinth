#!/usr/bin/python
#This script is intended not to be run manually but by GIMP as part of Cybrinth's build process.

import os

imageDir = "src/images"
compiledImageDir = "Images"

def convertImage( filePath ):
	image = pdb.gimp_file_load( filePath, filePath )
	fileNameParts = os.path.splitext( filePath )
	newFilePath = fileNameParts[ 0 ].replace( imageDir, compiledImageDir ) + ".png"
	#newFilePath = newFilePath.replace( ".xcf", ".xpm" )
	#newFilePath = newFilePath.replace( imageDir.lower(), compiledImageDir )
	
	newDirPath = os.path.dirname( newFilePath )
	if( not os.path.exists( newDirPath ) ):
		os.makedirs( newDirPath )
	
	#pdb.gimp_image_flatten( image )
	pdb.gimp_image_merge_visible_layers( image, CLIP_TO_IMAGE ) #This line is a workaround for a rare GIMP crashing bug. Saving to a format like XPM which does not support layers should cause GIMP to automatically merge all layers, which it does... 99.9999999% of the time.
	#pdb.gimp_file_save( image, image.active_drawable, newFilePath, newFilePath, run_mode=RUN_INTERACTIVE )
	#alphaThreshold = 127 #XPM format doesn't do alpha transparency; it does "invisible is a color that pixels can be"
	#pdb.file_xpm_save( image, image.active_drawable, newFilePath, newFilePath, alphaThreshold, run_mode=RUN_NONINTERACTIVE )
	
	pdb.file_png_save( image=image,  drawable=image.active_drawable, filename=newFilePath, raw_filename=newFilePath, interlace=True, compression=9, bkgd=False, gama=False, offs=False, phys=False, time=True, comment=True, svtrans=True )
	
	pdb.gimp_image_delete( image )

for roots, dirs, files in os.walk( imageDir ):
	for name in files:
		filePath = os.path.join( roots, name )
		if( filePath.lower().rfind( ".png" ) != -1 ):
			convertImage( filePath )


pdb.gimp_quit( TRUE )
