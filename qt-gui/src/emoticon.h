#ifndef zE1D68C95080DE073514FA90C07628F92
#define zE1D68C95080DE073514FA90C07628F92

#include <qstring.h>
#include <qstringlist.h>

class CEmoticons {

public:
	/*!
	 * \param basedir  base dir for icons
	 * \param theme    sets the current theme
	 */
	CEmoticons(const char *basedir, const char *theme = 0 );
	~CEmoticons();

	/*! \returns the list of themes available */
	QStringList Themes();
	
	/*!
	 * sets the current theme
	 * \retuns a negative number on error
	 */
	int SetTheme(const char *theme);
	
	/*! \returns the current theme */
	const char *Theme(void);
	
	/*! \returns the list of files of the current emoticon theme */
	QStringList fileList();
	
	/*! \returns the list of files for `theme` */
	QStringList fileList(const char *theme);
	
	void ParseMessage(QString &msg);

private:
	struct Emoticons *data;
};
#endif
