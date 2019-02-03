using Config;

namespace Buoh {
    public static int main(string [] argv) {
        Intl.bindtextdomain(Config.GETTEXT_PACKAGE, Config.LOCALE_DIR);
        Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE, "UTF-8");
        Intl.textdomain(Config.GETTEXT_PACKAGE);

        Buoh.Application buoh = new Buoh.Application();

        return buoh.run(argv);
    }
}
