Project Scope
-------------

  * Bug fixes and maintenance
  * Prioritize extensibility and simplicity
  * Do not make the codebase more complex, keep it simple to hack on
  * Do not add extra dependency (if we do, add compile time switch to disable it)
  * New features may be added if it cannot be achieved (easily) via a shell script,
    doesn't break backwards compatibility and doesn't violate any of the above rules.

Note: Since we aim to be a drop-in replacement for sxiv, we intend to keep all
sxiv's behaviors/features even in cases where removing them would make the
code-base simpler.


Contribution Guideline
----------------------

When contributing, make sure:

  * Your contribution falls under nsxiv's scope and aim
  * You follow the existing code style (see [.editorconfig](.editorconfig))
  * You open the pull request from a new branch, not from master
  * To avoid using force pushes, especially for bigger patches. Only use them
    when there's merge conflicts.

If your contribution is not suitable for general use, it will not be included in nsxiv.
For changes that are very much up to preference, such as changing values in config.h,
please do not open a pull request unless you have an objective explanation.

See the [open issues](https://github.com/nsxiv/nsxiv/issues) to find something
to work on. You can also filter the issues via label:

* [Good first issue](https://github.com/nsxiv/nsxiv/issues?q=is%3Aissue+is%3Aopen+label%3A%22good+first+issue%22):
  (Easy) Issues which do not require much if any experience.
* [Up for grabs](https://github.com/nsxiv/nsxiv/issues?q=is%3Aissue+is%3Aopen+label%3A%22up+for+grabs%22):
  (Intermediate) Issues which are free for anyone who wants to pick it up.
  Might require some experience.
* [Help wanted](https://github.com/nsxiv/nsxiv/issues?q=is%3Aissue+is%3Aopen+label%3A%22help+wanted%22):
  (Intermediate/Experienced) Issues where we require some help.

Development workflow for maintainers
------------------------------------

If we notice you contributing and/or showing interest in issues/pull requests,
we may invite you to join the nsxiv org as a member. Being a member simply means
you will be able to approve, disapprove and merge pull requests.

Our workflow regarding pull requests is the following:

  * Code related changes require two approvals, but documentation related
    changes (e.g. typo) can be merged with just one.
  * Always prefer squashing when merging. In the case a PR makes more than one
    significant change, use the "don't squash" tag and rebase instead.
  * When merging, make sure the commit message is cleaned up properly so that
    it reflects the current intention of the PR.

For releases, the process is the following:

  * Tag the release with a "vN" tag, where N is the version number. Also set
    the commit message and tag description for the release commit to "Release
    version N".
  * Update `VERSION` macro in the `Makefile`.
  * Update the changelog (`CHANGELOG.md`):
    * Include link to the release tarball and add the release date.
    * Document only the changes or fixes between releases. Don't document
      changes which never made it into a release.
    * Use the "Changes" section to document behavior changes since the last
      release, the "Added" section for new features, and the "Fixes" section
      for fixed bugs or regressions. Include pull request IDs.

For mundane development related talks which don't warrant their own issue, use
the [general-dev](https://github.com/nsxiv/nsxiv/discussions/119) discussion
thread.
