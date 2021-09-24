#include "ntios.h"
#include "drivers.h"
#include "texteditor.h"
#include "keys.h"

#include "drivers/graphics/graphics.h"

#include <stdbool.h>

void b_te_reRender(GraphicsDisplayDevice* gfx, int line, char* buffer, int bufend, char* filename) {
	bool willOverflow;
	char c;
	int nxline = 0, nxcolumn = 0;
	int maxlines = gfx->getTextLines() - 1; // minus one to account for top header
	int maxcolumns = gfx->getTextColumns();
	int isprinting = false;

	gfx->clearScreen();
	gfx->printf("te: Editing '%s'\t\tpress ESC to save+exit\n", filename);
	for (int i = 0; i < bufend; i++) {
		if (nxline >= line) isprinting = true;

		c = buffer[i];
		if (c == '\n' || c == '\r') {
			nxline++;
			nxcolumn = 0;
		} else if (c == '\t') {
			nxcolumn += 4 - (nxcolumn % 4);
		} else
			nxcolumn++;
		if (nxcolumn > maxcolumns) {
			nxcolumn = 0;
			nxline++;
		}
		willOverflow = (nxline - line >= maxlines);
		// are we out of lines?
		if (willOverflow) break;
		else if (isprinting) gfx->write(c);
	}
}

// This is here, but it appears that it is not used.
/*static bool incrementsLine(int col, int maxcol, char c) {
	if (c == '\n' || c == '\r') return true;
	else if (c == '\t') {
		col = (col % 4) + 4;
	} else
		col++;
	return col > maxcol;
}*/

int b_te(StreamDevice* io, int argc, char** argv) {
	// accessible via te command in terminal
	GraphicsDisplayDevice* gfx = NULL;

	if (argc < 2) {
		io->println("te: usage: te [file]");
		io->println("\t\tFile will be created if it doesn't already exist.");
		return -1;
	}
	if (io->getType() != DEV_TYPE_GRAPHICS) {
		if (io->getType() == DEV_TYPE_JOINED_STREAM) {
			StreamDevice* tmp = ((JoinedStreamDevice*)io)->getOutput();
			if (tmp->getType() == DEV_TYPE_GRAPHICS)
				gfx = (GraphicsDisplayDevice*)tmp;
		}
	} else
		gfx = (GraphicsDisplayDevice*)io;
	
	if (gfx == NULL) {
		io->println("te: error: not a graphical terminal");
		return -300;
	}

	char* filename;
	int bufsz;
	int bufloc;
	int bufend;
	char* buffer;

	NTIOSFile* file = fsopen(argv[1], NTIOS_READ);

	if (file != NULL) {
		if (file->isDirectory()) {
			io->println("File is a directory.");
			return -302;
		}

		filename = file->name();
		buffer = (char*)malloc(bufsz = 256 + (bufend = file->size()));
		if (buffer == NULL) {
			io->println("Could not allocate file buffer");
			file->close();
			free(file);
			return -303;
		}

		for (int i = 0; file->available(); i++)
			buffer[i] = file->read();
		file->close();
		free(file);
	} else {
		// create new empty file
		buffer = (char*)malloc(bufsz = 256);
		bufend = 0;
		filename = argv[1];
	}

	int nxline = 0, nxcolumn = 0;
	int displayline = 0;
	int maxlines = gfx->getTextLines() - 1; // minus one to account for top header
	int maxcolumns = gfx->getTextColumns();
	bool cursor_s = false;
	bool do_wr;
	int c;

	b_te_reRender(gfx, displayline, buffer, bufend, filename);

	bufloc = 0;
	gfx->setTextCursor(0, 1);
	while (true) {
		while (!io->available()) {
			c = buffer[bufloc];
			if (c == '\n' || c == '\r') c = ' ';
			if (cursor_s)
				gfx->write(c);
			else
				gfx->write('_');
			gfx->setTextCursor(nxcolumn, nxline - displayline + 1);
			cursor_s = !cursor_s;
			yield();
		}
		c = buffer[bufloc];
		if (c == '\n' || c == '\r') c = ' ';
		gfx->write(c); // the cursor and file position must ALWAYS be synced.
		gfx->setTextCursor(nxcolumn, nxline - displayline + 1);

		do_wr = true;
		c = io->read();

		if (c == KEY_ESC) {
			gfx->clearScreen();
			break;
		} else if (c == KEY_RIGHT) {
			if (bufloc == bufend) continue;
			c = buffer[bufloc++];
			do_wr = false;
		} else if (c == KEY_LEFT) {
			if (bufloc == 0) continue;
			bufloc--;
			nxline = 0;
			nxcolumn = 0;
			for (int i = 0; i < bufloc; i++) {
				c = buffer[i];
				if (c == '\n' || c == '\r') {
					nxline++;
					nxcolumn = 0;
				} else if (c == '\t') {
					nxcolumn += 4 - (nxcolumn % 4);
				} else
					nxcolumn++;
				if (nxcolumn > maxcolumns) {
					nxcolumn = 0;
					nxline++;
				}
			}
			gfx->setTextCursor(nxcolumn, nxline - displayline + 1);
			continue;
		} else if (c == KEY_UP) {
			bufloc--;
			while (bufloc--) {
				c = buffer[bufloc];
				if (c == '\n' || c == '\r') break;
			}
			bufloc++;
			if (bufloc != 0) {
				if (nxline <= displayline) {
					displayline -= max(1, maxlines / 3);
					b_te_reRender(gfx, displayline, buffer, bufend, filename);
				} else if (nxline != displayline)
					nxline--;
			}
			nxline = 0;
			nxcolumn = 0;
			for (int i = 0; i < bufloc; i++) {
				c = buffer[i];
				if (c == '\n' || c == '\r') {
					nxline++;
					nxcolumn = 0;
				} else if (c == '\t') {
					nxcolumn += 4 - (nxcolumn % 4);
				} else
					nxcolumn++;
				if (nxcolumn > maxcolumns) {
					nxcolumn = 0;
					nxline++;
				}
			}
			gfx->setTextCursor(nxcolumn, nxline - displayline + 1);
			continue;
		} else if (c == KEY_DOWN) {
			while (bufloc < bufend) {
				c = buffer[bufloc++];
				if (c == '\n' || c == '\r') break;
			}
			/*if (bufloc != 0) {
				if (nxline <= displayline) {
					displayline -= max(1, maxlines / 3);
					b_te_reRender(gfx, displayline, buffer, bufend, filename);
				} else if (nxline != displayline)
					nxline--;
			}*/
			nxline = 0;
			nxcolumn = 0;
			for (int i = 0; i < bufloc; i++) {
				c = buffer[i];
				if (c == '\n' || c == '\r') {
					nxline++;
					nxcolumn = 0;
				} else if (c == '\t') {
					nxcolumn += 4 - (nxcolumn % 4);
				} else
					nxcolumn++;
				if (nxcolumn > maxcolumns) {
					nxcolumn = 0;
					nxline++;
				}
			}
		
			if (nxline - displayline >= maxlines) {
				displayline += max(1, maxlines / 3);
				b_te_reRender(gfx, displayline, buffer, bufend, filename);
			}
		
			gfx->setTextCursor(nxcolumn, nxline - displayline + 1);
			continue;
		} else if (c == KEY_DELETE) {
			if (bufloc == bufend) continue;
			memmove(buffer + bufloc, buffer + bufloc + 1, bufend - bufloc);
			b_te_reRender(gfx, displayline, buffer, bufend - 1, filename);
			bufend--;
			continue;
		} else if (c == KEY_BACKSPACE) {
			if (bufloc == 0) continue;
			else if (bufloc != bufend) {
				char oldchar = buffer[bufloc - 1];
				memmove(buffer + bufloc - 1, buffer + bufloc, bufend - bufloc + 1);
				if (buffer[bufloc - 1] != '\n' || oldchar == '\n' || nxcolumn == 0)
					b_te_reRender(gfx, displayline, buffer, bufend - 1, filename);
				else {
					gfx->setTextCursor(nxcolumn - 1, nxline - displayline + 1);
					gfx->write(' ');
				}
			}
			bufend--;
			bufloc--;
			nxline = 0;
			nxcolumn = 0;
			for (int i = 0; i < bufloc; i++) {
				c = buffer[i];
				if (c == '\n' || c == '\r') {
					nxline++;
					nxcolumn = 0;
				} else if (c == '\t') {
					nxcolumn += 4 - (nxcolumn % 4);
				} else
					nxcolumn++;
				if (nxcolumn > maxcolumns) {
					nxcolumn = 0;
					nxline++;
				}
			}
			gfx->setTextCursor(nxcolumn, nxline - displayline + 1);
			continue;
		}

		int previous_next_line = nxline;

		if (c == '\n' || c == '\r') {
			nxline++;
			nxcolumn = 0;
		} else if (c == '\t') {
			nxcolumn += 4 - (nxcolumn % 4);
		} else
			nxcolumn++;
		if (nxcolumn >= maxcolumns) {
			nxcolumn = 0;
			nxline++;
		}

		if (nxline - displayline >= maxlines) {
			displayline += max(1, maxlines / 3);
			b_te_reRender(gfx, displayline, buffer, bufend, filename);
		}
		if (bufloc == bufend || !do_wr) {
			if (do_wr) {
				buffer[bufloc++] = c;

				// ensure we don't have a random char in the next spot for rendering
				buffer[bufloc] = ' ';

				bufend++;
			}
			gfx->write(c);
		} else {
			// insert...  so move the entire file over a byte and then write
			bufend++;
			if (bufend >= bufsz)
				buffer = (char*)realloc(buffer, bufsz += 128);
			memmove(buffer + bufloc + 1, buffer + bufloc, bufend - bufloc);
			buffer[bufloc++] = c;
			if (previous_next_line != nxline)
				b_te_reRender(gfx, displayline, buffer, bufend, filename);
			else if (do_wr)
				gfx->write(c);
			bufend++;
		}
		gfx->setTextCursor(nxcolumn, nxline - displayline + 1);
	}

	file = fsopen(argv[1], NTIOS_WRITE);
	for (int i = 0; i < bufend; i++)
		file->write(buffer[i]);
	file->close();
	free(file);
	free(buffer);

	io->println("[wrote file]");
	return 0;
}
