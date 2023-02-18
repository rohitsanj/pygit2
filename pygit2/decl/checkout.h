/**
 * Checkout behavior flags
 *
 * In libgit2, checkout is used to update the working directory and index
 * to match a target tree.  Unlike git checkout, it does not move the HEAD
 * commit for you - use `git_repository_set_head` or the like to do that.
 *
 * Checkout looks at (up to) four things: the "target" tree you want to
 * check out, the "baseline" tree of what was checked out previously, the
 * working directory for actual files, and the index for staged changes.
 *
 * You give checkout one of three strategies for update:
 *
 * - `GIT_CHECKOUT_NONE` is a dry-run strategy that checks for conflicts,
 *   etc., but doesn't make any actual changes.
 *
 * - `GIT_CHECKOUT_FORCE` is at the opposite extreme, taking any action to
 *   make the working directory match the target (including potentially
 *   discarding modified files).
 *
 * - `GIT_CHECKOUT_SAFE` is between these two options, it will only make
 *   modifications that will not lose changes.
 *
 *                         |  target == baseline   |  target != baseline  |
 *    ---------------------|-----------------------|----------------------|
 *     workdir == baseline |       no action       |  create, update, or  |
 *                         |                       |     delete file      |
 *    ---------------------|-----------------------|----------------------|
 *     workdir exists and  |       no action       |   conflict (notify   |
 *       is != baseline    | notify dirty MODIFIED | and cancel checkout) |
 *    ---------------------|-----------------------|----------------------|
 *      workdir missing,   | notify dirty DELETED  |     create file      |
 *      baseline present   |                       |                      |
 *    ---------------------|-----------------------|----------------------|
 *
 * To emulate `git checkout`, use `GIT_CHECKOUT_SAFE` with a checkout
 * notification callback (see below) that displays information about dirty
 * files.  The default behavior will cancel checkout on conflicts.
 *
 * To emulate `git checkout-index`, use `GIT_CHECKOUT_SAFE` with a
 * notification callback that cancels the operation if a dirty-but-existing
 * file is found in the working directory.  This core git command isn't
 * quite "force" but is sensitive about some types of changes.
 *
 * To emulate `git checkout -f`, use `GIT_CHECKOUT_FORCE`.
 *
 *
 * There are some additional flags to modify the behavior of checkout:
 *
 * - GIT_CHECKOUT_ALLOW_CONFLICTS makes SAFE mode apply safe file updates
 *   even if there are conflicts (instead of cancelling the checkout).
 *
 * - GIT_CHECKOUT_REMOVE_UNTRACKED means remove untracked files (i.e. not
 *   in target, baseline, or index, and not ignored) from the working dir.
 *
 * - GIT_CHECKOUT_REMOVE_IGNORED means remove ignored files (that are also
 *   untracked) from the working directory as well.
 *
 * - GIT_CHECKOUT_UPDATE_ONLY means to only update the content of files that
 *   already exist.  Files will not be created nor deleted.  This just skips
 *   applying adds, deletes, and typechanges.
 *
 * - GIT_CHECKOUT_DONT_UPDATE_INDEX prevents checkout from writing the
 *   updated files' information to the index.
 *
 * - Normally, checkout will reload the index and git attributes from disk
 *   before any operations.  GIT_CHECKOUT_NO_REFRESH prevents this reload.
 *
 * - Unmerged index entries are conflicts.  GIT_CHECKOUT_SKIP_UNMERGED skips
 *   files with unmerged index entries instead.  GIT_CHECKOUT_USE_OURS and
 *   GIT_CHECKOUT_USE_THEIRS to proceed with the checkout using either the
 *   stage 2 ("ours") or stage 3 ("theirs") version of files in the index.
 *
 * - GIT_CHECKOUT_DONT_OVERWRITE_IGNORED prevents ignored files from being
 *   overwritten.  Normally, files that are ignored in the working directory
 *   are not considered "precious" and may be overwritten if the checkout
 *   target contains that file.
 *
 * - GIT_CHECKOUT_DONT_REMOVE_EXISTING prevents checkout from removing
 *   files or folders that fold to the same name on case insensitive
 *   filesystems.  This can cause files to retain their existing names
 *   and write through existing symbolic links.
 */
typedef enum {
	GIT_CHECKOUT_NONE = 0, /**< default is a dry run, no actual updates */

	/**
	 * Allow safe updates that cannot overwrite uncommitted data.
	 * If the uncommitted changes don't conflict with the checked out files,
	 * the checkout will still proceed, leaving the changes intact.
	 *
	 * Mutually exclusive with GIT_CHECKOUT_FORCE.
	 * GIT_CHECKOUT_FORCE takes precedence over GIT_CHECKOUT_SAFE.
	 */
	GIT_CHECKOUT_SAFE = (1u << 0),

	/**
	 * Allow all updates to force working directory to look like index.
	 *
	 * Mutually exclusive with GIT_CHECKOUT_SAFE.
	 * GIT_CHECKOUT_FORCE takes precedence over GIT_CHECKOUT_SAFE.
	 */
	GIT_CHECKOUT_FORCE = (1u << 1),


	/** Allow checkout to recreate missing files */
	GIT_CHECKOUT_RECREATE_MISSING = (1u << 2),

	/** Allow checkout to make safe updates even if conflicts are found */
	GIT_CHECKOUT_ALLOW_CONFLICTS = (1u << 4),

	/** Remove untracked files not in index (that are not ignored) */
	GIT_CHECKOUT_REMOVE_UNTRACKED = (1u << 5),

	/** Remove ignored files not in index */
	GIT_CHECKOUT_REMOVE_IGNORED = (1u << 6),

	/** Only update existing files, don't create new ones */
	GIT_CHECKOUT_UPDATE_ONLY = (1u << 7),

	/**
	 * Normally checkout updates index entries as it goes; this stops that.
	 * Implies `GIT_CHECKOUT_DONT_WRITE_INDEX`.
	 */
	GIT_CHECKOUT_DONT_UPDATE_INDEX = (1u << 8),

	/** Don't refresh index/config/etc before doing checkout */
	GIT_CHECKOUT_NO_REFRESH = (1u << 9),

	/** Allow checkout to skip unmerged files */
	GIT_CHECKOUT_SKIP_UNMERGED = (1u << 10),
	/** For unmerged files, checkout stage 2 from index */
	GIT_CHECKOUT_USE_OURS = (1u << 11),
	/** For unmerged files, checkout stage 3 from index */
	GIT_CHECKOUT_USE_THEIRS = (1u << 12),

	/** Treat pathspec as simple list of exact match file paths */
	GIT_CHECKOUT_DISABLE_PATHSPEC_MATCH = (1u << 13),

	/** Ignore directories in use, they will be left empty */
	GIT_CHECKOUT_SKIP_LOCKED_DIRECTORIES = (1u << 18),

	/** Don't overwrite ignored files that exist in the checkout target */
	GIT_CHECKOUT_DONT_OVERWRITE_IGNORED = (1u << 19),

	/** Write normal merge files for conflicts */
	GIT_CHECKOUT_CONFLICT_STYLE_MERGE = (1u << 20),

	/** Include common ancestor data in diff3 format files for conflicts */
	GIT_CHECKOUT_CONFLICT_STYLE_DIFF3 = (1u << 21),

	/** Don't overwrite existing files or folders */
	GIT_CHECKOUT_DONT_REMOVE_EXISTING = (1u << 22),

	/** Normally checkout writes the index upon completion; this prevents that. */
	GIT_CHECKOUT_DONT_WRITE_INDEX = (1u << 23),

	/**
	 * Show what would be done by a checkout.  Stop after sending
	 * notifications; don't update the working directory or index.
	 */
	GIT_CHECKOUT_DRY_RUN = (1u << 24),

	/** Include common ancestor data in zdiff3 format for conflicts */
	GIT_CHECKOUT_CONFLICT_STYLE_ZDIFF3 = (1u << 25),

	/**
	 * THE FOLLOWING OPTIONS ARE NOT YET IMPLEMENTED
	 */

	/** Recursively checkout submodules with same options (NOT IMPLEMENTED) */
	GIT_CHECKOUT_UPDATE_SUBMODULES = (1u << 16),
	/** Recursively checkout submodules if HEAD moved in super repo (NOT IMPLEMENTED) */
	GIT_CHECKOUT_UPDATE_SUBMODULES_IF_CHANGED = (1u << 17)

} git_checkout_strategy_t;

typedef enum {
	GIT_CHECKOUT_NOTIFY_NONE      = 0,
	GIT_CHECKOUT_NOTIFY_CONFLICT  = 1,
	GIT_CHECKOUT_NOTIFY_DIRTY     = 2,
	GIT_CHECKOUT_NOTIFY_UPDATED   = 4,
	GIT_CHECKOUT_NOTIFY_UNTRACKED = 8,
	GIT_CHECKOUT_NOTIFY_IGNORED   = 16,

	GIT_CHECKOUT_NOTIFY_ALL       = 0x0FFFF
} git_checkout_notify_t;

typedef int (*git_checkout_notify_cb)(
	git_checkout_notify_t why,
	const char *path,
	const git_diff_file *baseline,
	const git_diff_file *target,
	const git_diff_file *workdir,
	void *payload);

typedef void (*git_checkout_progress_cb)(
	const char *path,
	size_t completed_steps,
	size_t total_steps,
	void *payload);

typedef struct {
	size_t mkdir_calls;
	size_t stat_calls;
	size_t chmod_calls;
} git_checkout_perfdata;

typedef void (*git_checkout_perfdata_cb)(
	const git_checkout_perfdata *perfdata,
	void *payload);

typedef struct git_checkout_options {
	unsigned int version;

	unsigned int checkout_strategy;

	int disable_filters;
	unsigned int dir_mode;
	unsigned int file_mode;
	int file_open_flags;

	unsigned int notify_flags;
	git_checkout_notify_cb notify_cb;
	void *notify_payload;

	git_checkout_progress_cb progress_cb;
	void *progress_payload;

	git_strarray paths;

	git_tree *baseline;

	git_index *baseline_index;

	const char *target_directory;

	const char *ancestor_label;
	const char *our_label;
	const char *their_label;

	git_checkout_perfdata_cb perfdata_cb;
	void *perfdata_payload;
} git_checkout_options;


int git_checkout_init_options(
	git_checkout_options *opts,
	unsigned int version);

int git_checkout_tree(
	git_repository *repo,
	const git_object *treeish,
	const git_checkout_options *opts);

int git_checkout_head(
	git_repository *repo,
	const git_checkout_options *opts);

int git_checkout_index(
	git_repository *repo,
	git_index *index,
	const git_checkout_options *opts);

