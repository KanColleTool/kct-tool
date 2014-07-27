if(UNIX AND NOT APPLE)
	install(FILES "KanColleTool.desktop" DESTINATION "share/applications" RENAME "kancolletool.desktop")
	install(FILES "KanColleTool.png" DESTINATION "share/icons/hicolor/192x192/apps" RENAME "kancolletool.png")
endif()
