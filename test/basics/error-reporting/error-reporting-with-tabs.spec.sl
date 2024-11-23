// prettier-ignore-file
import File
fn lol {
	const this_file_has_tabs = File
		.read(File.join_path(cwd(), "error-reporting-with-tabs.spec.sl"))
		.split("")
		.some(fn (c) -> c == "\t")

	print this_file_has_tabs // [expect] true
}

fn main {
	throw lol()	// [expect-error] Uncaught error: nil
}							// [expect-error]     13 | 	throw lol()
							// [expect-error]           ~~~~~~~~~~~
main()				// [expect-error]   at line 13 at the toplevel of module "main"
							// [expect-error]   at line 16 at the toplevel of module "main"

// What did we learn here? You should use spaces instead of tabs in your code.
