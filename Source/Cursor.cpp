#include "Cursor.h"

Cursor::Cursor() {
	
}

Cursor::Cursor(glm::vec3 v) {
	center_cursor_position = v;
}

void Cursor::updateCenterPositionCursor(glm::vec3 v){
	center_cursor_position = v;
}

glm::vec3 Cursor::getCenterPositionCursor(){
	return center_cursor_position;
}
