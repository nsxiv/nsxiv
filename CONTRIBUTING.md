Contribution Guideline
----------------------

The scope and aim of nsxiv are:

  * Bug fixes and maintenance
  * Prioritize extensibility and simplicity
  * Do not make the codebase more complex, keep it simple to hack on
  * Do not add extra dependency (if we do, add compile time switch to disable it)
  * New features may be added if it cannot be achieved (easily) via a shell script,
    doesn't break backwards compatibility and doesn't violate any of the above rules.
  
When contributing, make sure:

  * Your contribution falls under nsxiv's scope and aim
  * You follow the existing code style (see [.editorconfig](.editorconfig))
  * You open the pull request from a new branch, not from master

If your contribution is not suitable for general use, it will not be included in nsxiv.
For changes that are very much up to preference, such as changing values in config.h,
please do not open a pull request unless you have an objective explanation.

See [TODO.md](TODO.md) or the [open issues](https://github.com/nsxiv/nsxiv/issues) to find something to work on.
