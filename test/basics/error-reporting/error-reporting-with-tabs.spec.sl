// prettier-ignore-file
import File
fn lol {
	const this_file_has_tabs = File
		.read(File.join_path(cwd(), "error-reporting-with-tabs.spec.sl"))
		.split("")
		.some(fn (c) -> c == "\t")

	print this_file_has_tabs // [Expect] true
}

fn main {
	throw lol()	// [ExpectError] Uncaught error: nil
}							// [ExpectError]     13 | 	throw lol()
							// [ExpectError]           ~~~~~~~~~~~
main()				// [ExpectError]   at line 13 at the toplevel of module "main"
							// [ExpectError]   at line 16 at the toplevel of module "main"

// What did we learn here? You should use spaces instead of tabs in your code.
