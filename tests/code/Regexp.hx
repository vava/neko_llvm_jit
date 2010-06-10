package tests.code;

class Regexp {
	static public function main() {
		if (~/ab.*ac/.match("wreab12345frsfsacerw")) {
			neko.Lib.print("OK");
		} else {
			neko.Lib.print("Not OK");
		}
	}
}