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

Also note that (n)sxiv uses `imlib2` for loading images. Thus any request or
patches for adding support for new image formats should go into
[imlib2's repo](https://git.enlightenment.org/old/legacy-imlib2) instead.


Contribution Guideline
----------------------

When contributing, make sure:

  * Your contribution falls under nsxiv's scope and aim
  * You follow the existing code style (see [.editorconfig](../.editorconfig))
  * You open the pull request from a new branch, not from master
  * To avoid using force pushes, especially for bigger patches. Only use them
    when there's merge conflicts.

If your contribution is not suitable for general use, it will not be included in nsxiv.
For changes that are very much up to preference, such as changing values in config.h,
please do not open a pull request unless you have an objective explanation.

See the [open issues](https://codeberg.org/nsxiv/nsxiv/issues) to find something
to work on. You can also filter the issues via label:

* [Good first issue](https://codeberg.org/nsxiv/nsxiv/issues?labels=49698):
  (Easy) Issues which do not require much if any experience.
* [Up for grabs](https://codeberg.org/nsxiv/nsxiv/issues?labels=49705):
  (Intermediate) Issues which are free for anyone who wants to pick it up.
  Might require some experience.
* [Help wanted](https://codeberg.org/nsxiv/nsxiv/issues?labels=49699):
  (Intermediate/Experienced) Issues where we require some help.


Development workflow for maintainers
------------------------------------

If we notice you contributing and/or showing interest in issues/pull requests,
we may invite you to join the nsxiv org as a member. Being a member simply means
you will be able to approve, disapprove and merge pull requests.

Our workflow regarding pull requests is the following:

  * Code related changes require two approvals, but documentation related
    changes (e.g. typo) can be merged with just one.
  * If a pull request has a single approval, no objections and has been open
    for more than 7 days, then it may be force-merged.
  * Always prefer squashing when merging. In the case a PR makes more than one
    significant change, use the "don't squash" tag and rebase instead.
  * When merging, make sure the commit message is cleaned up properly so that
    it reflects the current intention of the PR.

For releases, the process is the following:

  * Tag the release with a "vN" tag, where N is the version number. Also set
    the commit message and tag description for the release commit to "Release
    version N". Make sure to use an annotated tag.
  * Update `VERSION` macro in `config.mk`.
  * Update the changelog (`etc/CHANGELOG.md`):
    * Include link to the release tarball and add the release date.
    * Document only the changes or fixes between releases. Don't document
      changes which never made it into a release.
    * Use the "Changes" section to document behavior changes since the last
      release, the "Added" section for new features, and the "Fixes" section
      for fixed bugs or regressions.
    * Include pull request IDs with reference style links.

Mirroring to GitHub:

Assuming `origin` is the name of the codeberg remote and `github` is the name
of the github remote; run the following commands to mirror the codeberg repo to
github:

```console
$ git fetch --prune origin
$ git push --prune github '+refs/remotes/origin/*:refs/heads/*' '+refs/tags/*:refs/tags/*'
```

The first command updates the local repo and the second command pushes
everything on `origin` without pushing any of the local branches.

- - -

For mundane development related talks which don't warrant their own issue, use
the [general discussion](https://codeberg.org/nsxiv/nsxiv/issues/294) thread.
