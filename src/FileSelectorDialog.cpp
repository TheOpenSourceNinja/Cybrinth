//This file originally downloaded from http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=30241&start=30&hilit=file+save+dialog. The copyright statement and license below do not apply to that original version, only to the modified version contained here.
/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 * 
 * @section LICENSE
 * Copyright © 2012-2014.
 * This file is part of Cybrinth.
 *
 * Cybrinth is free software: you can redistribute it and/or modify it under the terms of the GNU Affero General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * Cybrinth is distributed in the hope that it will be fun, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along with Cybrinth. If not, see <http://www.gnu.org/licenses/>.
 * 
 * @section DESCRIPTION
 * The FileSelectorDialog class allows the user to choose where to save or load a file and with which file name.
 */

#include "FileSelectorDialog.h"
#include "StringConverter.h"
#include <iostream>

const irr::s32 FOD_WIDTH = 350;
const irr::s32 FOD_HEIGHT = 265;

irr::s32 FileSelectorDialog::numFileSelectors = 0;

//! constructor
FileSelectorDialog::FileSelectorDialog( const irr::core::stringw title, irr::gui::IGUIEnvironment* environment, IGUIElement* parent, irr::s32 id, E_FILESELECTOR_TYPE type ) : IGUIFileOpenDialog( environment, parent, id, irr::core::rect< irr::s32 >( ( parent->getAbsolutePosition().getWidth() - FOD_WIDTH ) / 2, ( parent->getAbsolutePosition().getHeight() - FOD_HEIGHT ) / 2, ( parent->getAbsolutePosition().getWidth() - FOD_WIDTH) / 2 + FOD_WIDTH, ( parent->getAbsolutePosition().getHeight() - FOD_HEIGHT ) / 2 + FOD_HEIGHT ) ), Dragging( false ), FileNameText( 0 ), FileList( 0 ), DialogType( type ) {
	#ifdef _DEBUG
		IGUIElement::setDebugName("FileSelectorDialog");
	#endif   
	
	Text = title;
	IsDirectoryChoosable = false;
	
	irr::gui::IGUISkin* skin = Environment->getSkin();
	irr::gui::IGUISpriteBank* sprites = 0;
	irr::video::SColor color( 255, 255, 255, 255 );
	if ( skin ) {
		sprites = skin->getSpriteBank();
		color = skin->getColor( irr::gui::EGDC_WINDOW_SYMBOL );
	}
	
	irr::s32 buttonw = Environment->getSkin()->getSize( irr::gui::EGDS_WINDOW_BUTTON_WIDTH );
	irr::s32 posx = RelativeRect.getWidth() - buttonw - 4;
	
	CloseButton = Environment->addButton( irr::core::rect< irr::s32 >( posx, 3, posx + buttonw, 3 + buttonw ), this, -1, L"", L"Close");
	CloseButton->setSubElement( true );
	if ( sprites ) {
		CloseButton->setSpriteBank( sprites );
		CloseButton->setSprite( irr::gui::EGBS_BUTTON_UP, skin->getIcon( irr::gui::EGDI_WINDOW_CLOSE ), color );
		CloseButton->setSprite( irr::gui::EGBS_BUTTON_DOWN, skin->getIcon( irr::gui::EGDI_WINDOW_CLOSE ), color);
	}
	CloseButton->setAlignment( irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	CloseButton->grab();
	
	OKButton = Environment->addButton( irr::core::rect< irr::s32 >( RelativeRect.getWidth() - 80, 30, RelativeRect.getWidth() - 10, 50 ), this, -1, ( DialogType == EFST_OPEN_DIALOG ? L"Open" : L"Save" ) );
	OKButton->setSubElement( true );
	OKButton->setAlignment( irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	OKButton->grab();
	
	CancelButton = Environment->addButton( irr::core::rect< irr::s32 >( RelativeRect.getWidth() - 80, 55, RelativeRect.getWidth() - 10, 75 ),	this, -1, skin ? skin->getDefaultText( irr::gui::EGDT_MSG_BOX_CANCEL ) : L"Cancel" );
	CancelButton->setSubElement( true );
	CancelButton->setAlignment( irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	CancelButton->grab();
	
	FileBox = Environment->addListBox( irr::core::rect< irr::s32 >( 10, 80, RelativeRect.getWidth() - 90, 230 ), this, -1, true );
	FileBox->setSubElement( true );
	FileBox->setAlignment( irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_LOWERRIGHT );
	FileBox->grab();
	
	DriveBox = Environment->addComboBox( irr::core::rect< irr::s32 >( 10, 55, RelativeRect.getWidth() - 90, 75 ), this, -1 );
	DriveBox->setSubElement( true );
	DriveBox->setAlignment( irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	DriveBox->grab();   
	
	FileNameText = Environment->addEditBox( 0, irr::core::rect< irr::s32 >( 10, 30, RelativeRect.getWidth() - 90, 50 ), true, this, -1 );
	FileNameText->setSubElement( true );
	FileNameText->setAlignment( irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	FileNameText->setTextAlignment( irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	FileNameText->grab();
	
	FilterComboBox = Environment->addComboBox( irr::core::rect< irr::s32 >( 10, RelativeRect.getHeight() - 30, RelativeRect.getWidth() - 90, RelativeRect.getHeight() - 10), this, -1 );
	FilterComboBox->setSubElement( true );
	FilterComboBox->setAlignment( irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_LOWERRIGHT, irr::gui::EGUIA_UPPERLEFT, irr::gui::EGUIA_UPPERLEFT );
	FilterComboBox->grab();
	FilterComboBox->addItem( L"All Files" );
	
	irr::core::stringw str = "FileSelectorIcons";
	str += numFileSelectors++;
	SpriteBank = Environment->addEmptySpriteBank( str.c_str() );
	if ( SpriteBank ) {
		SpriteBank->grab();
		FileBox->setSpriteBank( SpriteBank );
	}
	DirectoryIconIdx = -1;
	FileIconIdx = -1;
	
	FileSystem = Environment->getFileSystem();
	
	if ( FileSystem ) {
		FileSystem->grab();
		prev_working_dir = FileSystem->getWorkingDirectory();
		//printf("working directory saved: %s\n", prev_working_dir.c_str());
	}
	
	fillListBox();
	
	
	#ifdef WINDOWS
		enum { SZ = 1024, GB = 1024*1024*1024 } ;
		char drives[SZ] ;
		
		if ( GetLogicalDriveStrings( SZ, drives ) < SZ ) {
			char* p = drives;
			while( *p ) { // two null chars; end of list
				StringConverter sc;
				DriveBox->addItem( sc.toSTDWString( p ) );
				
				while( *p ) {
					++p; // get to next null char
				}
				++p ; // and then skip over it
			}
		}
	#endif
	#ifndef WINDOWS
	
	#endif
	
}

//! destructor
FileSelectorDialog::~FileSelectorDialog() {
	if ( CloseButton ) {
		CloseButton->drop();
	}
	
	if ( OKButton ) {
		OKButton->drop();
	}
	
	if ( CancelButton ) {
		CancelButton->drop();
	}
	
	if ( FileBox ) {
		FileBox->drop();
	}
	
	if ( FileNameText ) {
		FileNameText->drop();
	}
	
	if ( FileSystem ) {
		FileSystem->drop();
	}
	
	if ( FileList ) {
		FileList->drop();
	}
	
	if ( FilterComboBox ) {
		FilterComboBox->drop();
	}
	
	if ( SpriteBank ) {
		SpriteBank->drop();
	}
	
}

//! returns the filename of the selected file. Returns NULL, if no file was selected.
const wchar_t* FileSelectorDialog::getFileName() const {
	return FileNameText->getText();
}

const irr::io::path& FileSelectorDialog::getDirectoryName() {
	FileSystem->flattenFilename ( FileDirectory );
	return FileDirectory;
}

//! called if an event happened.
bool FileSelectorDialog::OnEvent( const irr::SEvent& event ) {
	switch( event.EventType ) {
		case irr::EET_KEY_INPUT_EVENT: {
			switch ( event.KeyInput.Key ) {
				case irr::KEY_RETURN: {
					if ( FileSystem ) {
						FileSystem->changeWorkingDirectoryTo( irr::core::stringw( FileNameText->getText() ).c_str() );
						fillListBox();
						FileNameText->setText( irr::core::stringw( FileSystem->getWorkingDirectory() ).c_str() );
					}
					return true;
				}
				default: {
					break;
				}
			}
			break;
		}
		case irr::EET_GUI_EVENT: {
			switch( event.GUIEvent.EventType ) {
				case irr::gui::EGET_COMBO_BOX_CHANGED: {
					if ( event.GUIEvent.Caller == FilterComboBox ) {
						fillListBox();
					} else { //change drive
						if ( FileSystem ) {	 
							FileSystem->changeWorkingDirectoryTo( irr::core::stringw( DriveBox->getText() ).c_str() );
							fillListBox();
						}
					}
					break;
				}
				case irr::gui::EGET_ELEMENT_FOCUS_LOST: {
					Dragging = false;
					break;
				}
				case irr::gui::EGET_BUTTON_CLICKED: {
					if ( event.GUIEvent.Caller == CloseButton || event.GUIEvent.Caller == CancelButton ) {
						if ( FileSystem ) {
							FileSystem->changeWorkingDirectoryTo( prev_working_dir.c_str() );
							//printf("working directory reset to: %s\n", prev_working_dir.c_str());
						}
						sendCancelEvent();
						remove();
						return true;
					} else if ( event.GUIEvent.Caller == OKButton ) {
						if( IsDirectoryChoosable || matchesFileFilter( FileNameText->getText() ) ) {
							if ( FileSystem ) {
								FileSystem->changeWorkingDirectoryTo( prev_working_dir.c_str() );
								//printf("working directory reset to: %s\n", prev_working_dir.c_str());
							}
							sendSelectedEvent();
							remove();
							return true;
						}
					}
					break;
				}
				case irr::gui::EGET_LISTBOX_CHANGED: {
					irr::s32 selected = FileBox->getSelected();
					if( FileList && FileSystem ) {
						irr::core::stringw strw;
						strw = FileSystem->getWorkingDirectory();
						#ifdef WINDOWS
							if( strw[ strw.size() - 1 ] != L'\\' ) {
								strw += L"\\";
							}
						#else
							if( strw[ strw.size() - 1 ] != L'/' ) {
								strw += L"/";
							}
						#endif
						strw += FileBox->getListItem( selected );
						FileNameText->setText( strw.c_str() );
					}
					break;
				}
				case irr::gui::EGET_LISTBOX_SELECTED_AGAIN: {	  
					irr::s32 selected = FileBox->getSelected();
					if ( FileList && FileSystem ) {
						if ( FileList->isDirectory( selected ) ) {
							FileSystem->changeWorkingDirectoryTo( FileList->getFileName( selected ) );
							fillListBox();
							FileNameText->setText( irr::core::stringw( FileSystem->getWorkingDirectory() ).c_str() );
						} else {
							irr::core::stringw strw;
							strw = FileSystem->getWorkingDirectory();
							#ifdef WINDOWS
								if( strw[ strw.size() - 1 ] != L'\\' ) {
									strw += L"\\";
								}
							#else
								if( strw[ strw.size() - 1 ] != L'/' ) {
									strw += L"/";
								}
							#endif
							strw += FileBox->getListItem( selected );
							FileNameText->setText( strw.c_str() );
							return true;
						}
					}
					break;
				}
				default: {
					break;
				}
			}
			break;
		}
		case irr::EET_MOUSE_INPUT_EVENT: {
			switch( event.MouseInput.Event ) {
				case irr::EMIE_LMOUSE_PRESSED_DOWN: {
					DragStart.X = event.MouseInput.X;
					DragStart.Y = event.MouseInput.Y;
					Dragging = true;
					Environment->setFocus( this );
					return true;
				}
				case irr::EMIE_LMOUSE_LEFT_UP: {
					Dragging = false;
					Environment->removeFocus( this );
					return true;
				}
				case irr::EMIE_MOUSE_MOVED: {
					if ( Dragging ) {
						// gui window should not be dragged outside its parent
						if ( Parent ) {
							if ( event.MouseInput.X < Parent->getAbsolutePosition().UpperLeftCorner.X + 1 || event.MouseInput.Y < Parent->getAbsolutePosition().UpperLeftCorner.Y + 1 || event.MouseInput.X > Parent->getAbsolutePosition().LowerRightCorner.X - 1 || event.MouseInput.Y > Parent->getAbsolutePosition().LowerRightCorner.Y -1 ) {
								return true;
							}
						}
						move( irr::core::position2d< irr::s32 >( event.MouseInput.X - DragStart.X, event.MouseInput.Y - DragStart.Y ) );
						DragStart.X = event.MouseInput.X;
						DragStart.Y = event.MouseInput.Y;
						return true;
					}
					break;
				}
				default: {
					break;
				}
			}
		}
		default: {
			break;
		}
	}
	
	return Parent ? Parent->OnEvent( event ) : false;
}

//! draws the element and its children
void FileSelectorDialog::draw() {
	if ( !IsVisible ) {
		return;
	}
	
	irr::gui::IGUISkin* skin = Environment->getSkin();
	
	irr::core::rect< irr::s32 > rect = AbsoluteRect;
	
	rect = skin->draw3DWindowBackground( this, true, skin->getColor( irr::gui::EGDC_ACTIVE_BORDER ), rect, &AbsoluteClippingRect );
	
	if ( Text.size() ) {
		rect.UpperLeftCorner.X += 2;
		rect.LowerRightCorner.X -= skin->getSize( irr::gui::EGDS_WINDOW_BUTTON_WIDTH ) + 5;
		
		irr::gui::IGUIFont* font = skin->getFont( irr::gui::EGDF_WINDOW );
		if ( font ) {
			font->draw( Text.c_str(), rect, skin->getColor( irr::gui::EGDC_ACTIVE_CAPTION ), false, true, &AbsoluteClippingRect);
		}
	}
	
	IGUIElement::draw();
}

bool FileSelectorDialog::matchesFileFilter( irr::core::stringw s ) {
	if ( FileFilters.size() > 1 ) {
		irr::s32 selected = FilterComboBox->getSelected();
		if ( selected == 0 ) {
			for ( irr::u32 i = 0; i < FileFilters.size(); i++ ) {
				irr::s32 pos = s.findLast( L'.' ) + 1; // Find the last '.' so we can check the file extension
				irr::core::stringw extension( s.subString( pos, s.size() - pos ) );
				if ( extension.equals_ignore_case( FileFilters[ FilterComboBox->getSelected() ].FileExtension ) ) {
					return true;
				}
			}
			return false;
		}
		selected--;
		if ( selected >= ( irr::s32 )FileFilters.size() ) {
			return true; // 'All Files' selectable
		} else {
			/*irr::s32 pos = s.findLast( L'.' ) + 1; // Find the last '.' so we can check the file extension
			irr::core::stringw extension( s.subString( pos, s.size() - pos ) );
			return extension.equals_ignore_case( FileFilters[ FilterComboBox->getSelected() ].FileExtension );*/
			return matchesFileFilter( s, FileFilters[ FilterComboBox->getSelected() ].FileExtension );
		}
	}
	if ( FilterComboBox->getSelected() >= ( irr::s32 )FileFilters.size() ) {
		return true; // 'All Files' selectable
	} else {
		/*irr::s32 pos = s.findLast( L'.' ) + 1; // Find the last '.' so we can check the file extension
		irr::core::stringw extension( s.subString( pos, s.size() - pos ) );
		return extension.equals_ignore_case( FileFilters[ FilterComboBox->getSelected() ].FileExtension );*/
		return matchesFileFilter( s, FileFilters[ FilterComboBox->getSelected() ].FileExtension );
	}
}

bool FileSelectorDialog::matchesFileFilter( irr::core::stringw s, irr::core::stringw f ) {
	irr::s32 pos = s.findLast( L'.' ) + 1; // Find the last '.' so we can check the file extension
	irr::core::stringw extension( s.subString( pos, s.size() - pos ) );
	return extension.equals_ignore_case( f );
}

//! fills the listbox with files.
void FileSelectorDialog::fillListBox() {
	irr::gui::IGUISkin *skin = Environment->getSkin();
	
	if ( !FileSystem || !FileBox || !skin ) {
		return;
	}
	
	if ( FileList ) {
		FileList->drop();
	}
	
	FileBox->clear();
	
	FileList = FileSystem->createFileList();
	irr::core::stringw s;
	
	for ( irr::u32 i = 0; i < FileList->getFileCount(); ++i ) {
		s = FileList->getFileName( i );
		// We just want a list of directories and those matching the file filter
		if ( FileList->isDirectory( i ) ) {
			if ( DirectoryIconIdx != -1 ) {
				FileBox->addItem( s.c_str(), DirectoryIconIdx );
			} else {
				FileBox->addItem( s.c_str() );
			}
		} else if ( matchesFileFilter( s ) ) {
			if ( FilterComboBox->getSelected() >= ( irr::s32 )FileFilters.size() ) {
				if ( FileIconIdx != -1 ) {
					irr::s32 iconIdx = FileIconIdx;
					for ( irr::u32 i = 0 ; i < FileFilters.size() ; i++ ) {
						if ( matchesFileFilter( s, FileFilters[ i ].FileExtension) ) {
							iconIdx = FileFilters[ i ].FileIconIdx;
						}
					}
					FileBox->addItem( s.c_str(), iconIdx );
				} else {
					FileBox->addItem( s.c_str() );
				}
			} else {
				FileBox->addItem( s.c_str(), FileFilters[ FilterComboBox->getSelected() ].FileIconIdx );	
			}
		}
	}

	if ( FileNameText ) {
		s = FileSystem->getWorkingDirectory();
		FileNameText->setText( s.c_str() );
	}
}


//! sends the event that the file has been selected.
void FileSelectorDialog::sendSelectedEvent() {
	irr::SEvent event;
	event.EventType = irr::EET_GUI_EVENT;
	event.GUIEvent.Caller = this;
	event.GUIEvent.EventType = irr::gui::EGET_FILE_SELECTED;
	Parent->OnEvent( event );
}

//! sends the event that the file choose process has been cancelled
void FileSelectorDialog::sendCancelEvent() {
	irr::SEvent event;
	event.EventType = irr::EET_GUI_EVENT;
	event.GUIEvent.Caller = this;
	event.GUIEvent.EventType = irr::gui::EGET_FILE_CHOOSE_DIALOG_CANCELLED;
	Parent->OnEvent( event );
}

void FileSelectorDialog::addFileFilter( irr::core::stringw name, irr::core::stringw ext, irr::video::ITexture* texture ) {
	SFileFilter filter( name, ext, texture );
	
	filter.FileIconIdx = addIcon( texture );
	
	FileFilters.push_back( filter );
	
	FilterComboBox->clear();
	irr::core::stringw strw;
	
	if ( FileFilters.size() > 1 ) {
		strw = L"Supported ";
		for ( irr::u32 i = 0 ; i < FileFilters.size() ; i++ ) {
			strw += L".";
			strw += FileFilters[ i ].FileExtension;
			strw += L" ";
		}
		FilterComboBox->addItem(strw.c_str());
	}
	
	for ( irr::u32 i = 0 ; i < FileFilters.size() ; i++ ) {
		strw = FileFilters[ i ].FilterName;
		strw += L" (*.";
		strw += FileFilters[ i ].FileExtension;
		strw += L")";
		FilterComboBox->addItem( strw.c_str() );
	}
	FilterComboBox->addItem( L"All Files" );
	
	fillListBox();
}

irr::u32 FileSelectorDialog::addIcon( irr::video::ITexture* texture ) {
	if ( !SpriteBank || !texture ) {
		return 0;
	}
	
	// load and add the texture to the bank	
	SpriteBank->addTexture( texture );
	irr::u32 textureIndex = SpriteBank->getTextureCount() - 1;
	// now lets get the sprite bank's rectangles and add some for our animation
	irr::core::array< irr::core::rect< irr::s32 > >& rectangles = SpriteBank->getPositions();
	irr::u32 firstRect = rectangles.size();
	// remember that rectangles are not in pixels, they enclose pixels!
	// to draw a rectangle around the pixel at 0,0, it would rect<irr::s32>(0,0, 1,1)
	rectangles.push_back( irr::core::rect< irr::s32 >( 0, 0, 16, 16 ) );
	
	// now we make a sprite..
	irr::gui::SGUISprite sprite;
	sprite.frameTime = 30;
	// add some frames of animation.
	irr::gui::SGUISpriteFrame frame;
	frame.rectNumber = firstRect;
	frame.textureNumber = textureIndex;
	
	// add this frame
	sprite.Frames.push_back( frame );
	// add the sprite
	//irr::u32 spriteIndex = SpriteBank->getSprites().size();
	SpriteBank->getSprites().push_back( sprite ); 
	
	return textureIndex;
}
