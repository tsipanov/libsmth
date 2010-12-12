#include <smth-http.h>
#include <smth-manifest-parser.h>
#include <smth-common-defs.h>

int SMTH_open(const char *url, const char *params)
{
	FILE *mfile = SMTH_fetchmanifest(url, params);
	Manifest manifest;
	SMTH_parsemanifest(&manifest, mfile);
	fclose(mfile);

	SMTH_disposemanifest(&manifest);

	return 0;
}
