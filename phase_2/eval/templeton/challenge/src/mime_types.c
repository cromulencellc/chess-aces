#include <string.h>

char *extension_to_content_type( char *name )
{
	char *dot = NULL;

	if ( name == NULL ) {
		return NULL;
	}

	dot = strchr( name, '.');

	if ( dot == NULL ) {
		return "application/octet-stream";
	}

	dot++;

	if ( !strcmp( dot, "aac" ) ) {
		return "audio/aac";
	} else if ( !strcmp( dot, "abw" ) ) {
		return "application/x-abiword";
	} else if ( !strcmp( dot, "arc" ) ) {
		return "application/x-freearc";
	} else if ( !strcmp( dot, "avi" ) ) {
		return "video/x-msvideo";
	} else if ( !strcmp( dot, "azw" ) ) {
		return "application/vnd.amazon.ebook";
	} else if ( !strcmp( dot, "bin" ) ) {
		return "application/octet-stream";
	} else if ( !strcmp( dot, "bmp" ) ) {
		return "image/bmp";
	} else if ( !strcmp( dot, "bz" ) ) {
		return "application/x-bzip";
	} else if ( !strcmp( dot, "bz2" ) ) {
		return "application/x-bzip2";
	} else if ( !strcmp( dot, "csh" ) ) {
		return "application/x-csh";
	} else if ( !strcmp( dot, "css" ) ) {
		return "text/css";
	} else if ( !strcmp( dot, "csv" ) ) {
		return "text/csv";
	} else if ( !strcmp( dot, "doc" ) ) {
		return "application/msword";
	} else if ( !strcmp( dot, "docx" ) ) {
		return "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
	} else if ( !strcmp( dot, "eot" ) ) {
		return "application/vnd.ms-fontobject";
	} else if ( !strcmp( dot, "epub" ) ) {
		return "application/epub+zip";
	} else if ( !strcmp( dot, "gz" ) ) {
		return "application/gzip";
	} else if ( !strcmp( dot, "gif" ) ) {
		return "image/gif";
	} else if ( !strcmp( dot, "htm" ) ) {
		return "text/html";
	} else if ( !strcmp( dot, "html" ) ) {
		return "text/html";
	} else if ( !strcmp( dot, "ico" ) ) {
		return "image/vnd.microsoft.icon";
	} else if ( !strcmp( dot, "ics" ) ) {
		return "text/calendar";
	} else if ( !strcmp( dot, "jar" ) ) {
		return "application/java-archive";
	} else if ( !strcmp( dot, "jpeg" ) ) {
		return "image/jpeg";
	} else if ( !strcmp( dot, "jpg" ) ) {
		return "image/jpeg";
	} else if ( !strcmp( dot, "js" ) ) {
		return "text/javascript";
	} else if ( !strcmp( dot, "json" ) ) {
		return "application/json";
	} else if ( !strcmp( dot, "jsonld" ) ) {
		return "application/ld+json";
	} else if ( !strcmp( dot, "mid" ) ) {
		return "audio/midi";
	} else if ( !strcmp( dot, "midi" ) ) {
		return "audio/midi";
	} else if ( !strcmp( dot, "mjs" ) ) {
		return "text/javascript";
	} else if ( !strcmp( dot, "mp3" ) ) {
		return "audio/mpeg";
	} else if ( !strcmp( dot, "mpeg" ) ) {
		return "video/mpeg";
	} else if ( !strcmp( dot, "mpkg" ) ) {
		return "application/vnd.apple.installer+xml";
	} else if ( !strcmp( dot, "odp" ) ) {
		return "application/vnd.oasis.opendocument.presentation";
	} else if ( !strcmp( dot, "ods" ) ) {
		return "application/vnd.oasis.opendocument.spreadsheet";
	} else if ( !strcmp( dot, "odt" ) ) {
		return "application/vnd.oasis.opendocument.text";
	} else if ( !strcmp( dot, "oga" ) ) {
		return "audio/ogg";
	} else if ( !strcmp( dot, "ogv" ) ) {
		return "video/ogg";
	} else if ( !strcmp( dot, "ogx" ) ) {
		return "application/ogg";
	} else if ( !strcmp( dot, "opus" ) ) {
		return "audio/opus";
	} else if ( !strcmp( dot, "otf" ) ) {
		return "font/otf";
	} else if ( !strcmp( dot, "png" ) ) {
		return "image/png";
	} else if ( !strcmp( dot, "pdf" ) ) {
		return "application/pdf";
	} else if ( !strcmp( dot, "php" ) ) {
		return "application/x-httpd-php";
	} else if ( !strcmp( dot, "ppt" ) ) {
		return "application/vnd.ms-powerpoint";
	} else if ( !strcmp( dot, "pptx" ) ) {
		return "application/vnd.openxmlformats-officedocument.presentationml.presentation";
	} else if ( !strcmp( dot, "rar" ) ) {
		return "application/vnd.rar";
	} else if ( !strcmp( dot, "rtf" ) ) {
		return "application/rtf";
	} else if ( !strcmp( dot, "sh" ) ) {
		return "application/x-sh";
	} else if ( !strcmp( dot, "svg" ) ) {
		return "image/svg+xml";
	} else if ( !strcmp( dot, "swf" ) ) {
		return "application/x-shockwave-flash";
	} else if ( !strcmp( dot, "tar" ) ) {
		return "application/x-tar";
	} else if ( !strcmp( dot, "tif" ) ) {
		return "image/tiff";
	} else if ( !strcmp( dot, "tiff" ) ) {
		return "image/tiff";
	} else if ( !strcmp( dot, "ts" ) ) {
		return "video/mp2t";
	} else if ( !strcmp( dot, "ttf" ) ) {
		return "font/ttf";
	} else if ( !strcmp( dot, "txt" ) ) {
		return "text/plain";
	} else if ( !strcmp( dot, "vsd" ) ) {
		return "application/vnd.visio";
	} else if ( !strcmp( dot, "wav" ) ) {
		return "audio/wav";
	} else if ( !strcmp( dot, "weba" ) ) {
		return "audio/webm";
	} else if ( !strcmp( dot, "webm" ) ) {
		return "video/webm";
	} else if ( !strcmp( dot, "webp" ) ) {
		return "image/webp";
	} else if ( !strcmp( dot, "woff" ) ) {
		return "font/woff";
	} else if ( !strcmp( dot, "woff2" ) ) {
		return "font/woff2";
	} else if ( !strcmp( dot, "xhtml" ) ) {
		return "application/xhtml+xml";
	} else if ( !strcmp( dot, "xls" ) ) {
		return "application/vnd.ms-excel";
	} else if ( !strcmp( dot, "xlsx" ) ) {
		return "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
	} else if ( !strcmp( dot, "xml" ) ) {
		return "text/xml";
	} else if ( !strcmp( dot, "xul" ) ) {
		return "application/vnd.mozilla.xul+xml";
	} else if ( !strcmp( dot, "zip" ) ) {
		return "application/zip";
	} else if ( !strcmp( dot, "3gp" ) ) {
		return "video/3gpp";
	} else if ( !strcmp( dot, "3g2" ) ) {
		return "video/3gpp2";
	} else if ( !strcmp( dot, "7z" ) ) {
		return "application/x-7z-compressed";
	}
	
	return "application/octet-stream";
}