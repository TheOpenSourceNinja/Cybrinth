//This file originally downloaded from http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=30241&start=30&hilit=file+save+dialog. The copyright statement and license below do not apply to that original version, only to the modified version contained here.
/**
 * @file
 * @author James Dearing <dearingj@lifetime.oregonstate.edu>
 *
 * @section LICENSE
 * Copyright © 2012-2015.
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

#ifndef FILESELECTORDIALOG_H
#define FILESELECTORDIALOG_H

#ifdef WINDOWS
    #include <irrlicht.h>
#else
    #include <irrlicht/irrlicht.h>
#endif
#ifdef WINDOWS
	#include <windows.h>
	#include <iostream>
#endif

#include "PreprocessorCommands.h"
#include "Integers.h"

/** Class for opening/saving files. */
class FileSelectorDialog : public irr::gui::IGUIFileOpenDialog {
	public:
		/** Enum to specify the usage of the instance of the class */
		enum E_FILESELECTOR_TYPE : uint_fast8_t {
			EFST_OPEN_DIALOG, //<! For opening files
			EFST_SAVE_DIALOG, //<! For saving files
			EFST_NUM_TYPES    //<! Not used, just specifies how many possible types there are
		};

		/**
		\brief Constructor
		\param title - The title of the dialog
		\pararm environment - The GUI environment to be used
		\param parent - The parent of the dialog
		\param id - The ID of the dialog
		\param type - The type of dialog
		*/
		FileSelectorDialog( const irr::core::stringw  title, irr::gui::IGUIEnvironment* environment, irr::gui::IGUIElement* parent, irr::s32 id, E_FILESELECTOR_TYPE type );

		/**
		\brief Destructor
		*/
		virtual ~FileSelectorDialog();
		/**
		\brief Returns the filename of the selected file. Returns NULL, if no file was selected.
		\return a const wchar_t*
		*/
		virtual const wchar_t* getFileName() const;
		//! Returns the directory of the selected file. Returns NULL, if no directory was selected.
		virtual const irr::io::path& getDirectoryName();
		/**
		\brief called if an event happened.
		\param even - the event that happened
		\return a bool
		*/
		virtual bool OnEvent( const irr::SEvent& event );
		/**
		\brief Render function
		*/
		virtual void draw();
		/**
		\brief Returns the current file filter selected or "All Files" if no filter is applied
		\return a stringw
		*/
		inline irr::core::stringw getFileFilter() const {
			if ( FilterComboBox->getSelected() >= ( irr::s32 )FileFilters.size() ) return irr::core::stringw( L"All Files" );
			else return FileFilters[ FilterComboBox->getSelected() ].FileExtension;
		}

		/**
		\brief Returns the type of the dialog
		\return a E_FILESELECTOR_TYPE
		*/
		inline E_FILESELECTOR_TYPE getDialogType() { return DialogType; }

		/**
		\brief Add a file filter
		\param name - The description of the file type
		\param ext - The file's extension
		\param texture - The icon texture
		*/
		void addFileFilter( irr::core::stringw  name, irr::core::stringw  ext, irr::video::ITexture* texture );

		/**
		\brief Set an icon to use to display unknown files
		\param texture - the 16x16 icon to use
		*/
		inline void setCustomFileIcon( irr::video::ITexture* texture ) {
			if ( texture ) FileIconIdx = addIcon( texture );
			else FileIconIdx = -1;
			fillListBox();
		}
		/**
		\brief Set an icon to use to display directories
		\param texture - the 16x16 icon to use
		*/
		inline void setCustomDirectoryIcon( irr::video::ITexture* texture ) {
			if ( texture ) DirectoryIconIdx = addIcon( texture );
			else DirectoryIconIdx = -1;
			fillListBox();
		}

		/**
		\brief Sets whether directories can be chosen as the 'file' to open
		\param choosable - Whether the directory can be chosen
		*/
		inline void setDirectoryChoosable( bool choosable ) { IsDirectoryChoosable = choosable; }

	protected:

		/**
		\brief Returns true if the file extension is one of the registered filters
		\param s - the string to be checked
		\return a bool
		*/
		bool matchesFileFilter( irr::core::stringw s );
		/**
		\brief Returns true if the file extension matches the specified filter
		\param s - the string to be checked
		\param f - the filter to check for
		\return a bool
		*/
		bool matchesFileFilter( irr::core::stringw s, irr::core::stringw f );

		/**
		\brief Fills the listbox with files.
		*/
		void fillListBox();

		/**
		\brief Sends the event that the file has been selected.
		*/
		void sendSelectedEvent();

		/**
		\brief Sends the event that the file choose process has been canceld
		*/
		void sendCancelEvent();

		irr::u32 addIcon( irr::video::ITexture* texture );

		/** Struct to describe file filters to use when displaying files in directories */
		struct SFileFilter {
			/*
			\brief Constructor
			\param name - The name/description of the filter
			\param filter - The file extension required
			\param texture - The texture to use as an icon for the file type
			*/
			SFileFilter( irr::core::stringw  name, irr::core::stringw  filter, irr::video::ITexture* texture ) {
				FilterName = name;
				FileExtension = filter;
				FileIcon = texture;
				FileIconIdx = 0;
			}
			void operator=( const SFileFilter& other ) {
				FilterName = other.FilterName;
				FileExtension = other.FileExtension;
				FileIcon = other.FileIcon;
				FileIconIdx = other.FileIconIdx;
			}
			irr::core::stringw FilterName;
			irr::core::stringw FileExtension;
			irr::video::ITexture* FileIcon;
			irr::u32 FileIconIdx;
		};

		irr::core::position2d< irr::s32 > DragStart;
		bool Dragging;
		bool IsDirectoryChoosable;
		irr::s32 FileIconIdx;
		irr::s32 DirectoryIconIdx;
		irr::gui::IGUIButton* CloseButton;
		irr::gui::IGUIButton* OKButton;
		irr::gui::IGUIButton* CancelButton;
		irr::gui::IGUIEditBox* FileNameText;
		irr::gui::IGUIListBox* FileBox;
		irr::gui::IGUIComboBox* DriveBox;
		irr::gui::IGUIComboBox* FilterComboBox;
		irr::gui::IGUIElement* EventParent;
		irr::gui::IGUISpriteBank* SpriteBank;
		irr::io::IFileSystem* FileSystem;
		irr::io::IFileList* FileList;
		irr::core::array< SFileFilter > FileFilters;
		E_FILESELECTOR_TYPE DialogType;
		irr::core::stringc prev_working_dir;
		irr::io::path FileDirectory;

		static irr::s32 numFileSelectors;
};

#endif // FILESELECTORDIALOG_H
