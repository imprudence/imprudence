#!/usr/bin/env python
# 
# @file   make_changelog.py
# @brief  Generates Imprudence's ChangeLog.txt from Git commit messages.
#
# Copyright (c) 2010, Jacek Antonelli
#
# The source code in this file is provided to you under the terms of
# the GNU General Public License, version 2.0 ("GPL"). Terms of the
# GPL can be found in doc/GPL-license.txt in this distribution, or
# online at http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
#
# By copying, modifying or distributing this software, you acknowledge
# that you have read and understood your obligations described above,
# and agree to abide by those obligations.
#
# ALL SOURCE CODE IS PROVIDED "AS IS." THE AUTHOR MAKES NO
# WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
# COMPLETENESS OR PERFORMANCE.
# 


# This script generates ChangeLog.txt (in the the imprudence root
# directory) from the commit messages in the git repository. It parses
# the output from 'git log', then prints the log entries in a nicely
# formatted way, omitting unimportant or unwanted commits.
# 
# A commit will be omitted if any of the following are true:
# 
#   * It is listed in SPECIAL_COMMITS below with "-".
#   * It has "@nochangelog" in its commit message (but not
#     "@forcechangelog" or "@shortchangelog").
#   * It is a merge commit (i.e. has multiple parent commits).
#   * It is a plain SL source commit ("Second Life viewer sources ...").
# 
# But even if the above are true, it WILL be included if any of the
# following are true:
# 
#   * It is listed in SPECIAL_COMMITS below with "+" or "S".
#   * It has "@forcechangelog" or "@shortchangelog" in its commit
#     message (but not "@nochangelog").
# 
# By default, the ChangeLog will list the files that were modified,
# added, or deleted for each commit. But, that list will be omitted
# (or summarized) if any of the following are true:
# 
#   * More than 10 files were changed.
#   * It is listed in SPECIAL_COMMITS below with "S".
#   * It has "@shortchangelog" in its commit message.
# 


import re, os, sys, subprocess


SCRIPT_DIR = os.path.abspath( os.path.dirname( sys.argv[0] ) )
ROOT_DIR   = os.path.normpath( os.path.join(SCRIPT_DIR, "..", "..") )
CHANGELOG  = os.path.join(ROOT_DIR, "ChangeLog.txt")


# Commit IDs that should be treated specially.
#
# IDs with "+" will always be included, even if they would have been
# omitted for some other reason (such as being a merge commit).
#
# IDs with "-" will be omitted (equivalent to @nochangelog)
#
# IDs with "S" will always be included (like "+"), but will hide or
# summarize the list of changed files (equivalent to @shortchangelog)

SPECIAL_COMMITS = {

    # Merge commits that ARE worth mentioning.
    "13c27361136819af0d0a0f72eeb807f56829337f" : "+",
    "499afbab7be4c4136eea0e1897319c3ac4e9799d" : "+",
    "4bbee8774d743fb4787fc2435a20d89539011fa4" : "+",
    "6a5aab98892df74f60743f5b789959c9593d6647" : "+",
    "6e1e243da8c06ddd6847576c55e134d72ef42491" : "+",
    "87c760f959788e3ec9dc06cbd2207d0242b6a4c9" : "+",
    "8f50d81693ff9463ae49c36935977a2b70e6bf45" : "+",
    "94a57bea1504f2f7024264b15c3ed532d49d3ab0" : "+",
    "96aaf4408601768d1d277bd63e6a1c91295c4c5a" : "+",
    "9a4f5702473e7540cde1bbff2d7189d9ed71fd86" : "+",

    # Mention the first SL version Imprudence is based on (1.20.15)
    "31ba61e675d42675c6c6cd1077a93e0c5e055114" : "S",

    # Project early setup. Not worth mentioning.
    "6f0d1dc6b922f1b103a8933a03c4ed5e10b290ef" : "-",
    "993bca391adf825dcf139d516c17dfdca0832bc8" : "-",
    "e8ca04117d66356e8fe514d617d1da2d9b49a927" : "-",
    "ec8b17013071896e7228c7d0032d3bfc44697c3a" : "-",
    "f258e5d9af1087cab2be36a8c143815300187d62" : "-",
    "f37093d4dda55fd77bc9228c9fb359d46d4f1715" : "-",

    # Whitespace and line ending changes. Not worth mentioning.
    "285dcc2660ef0ed31bfbdc4f0a5cc53f40e90e36" : "-",
    "44dc53ef5d8f770412c2f7675b9ebfdeb3c2b698" : "-",
    "451aad0c993856f380d976de3d7fe343ad5f9811" : "-",
    "45beb0b1d8a16522dfc33894f4b69ee9a4b33efc" : "-",
    "567586af40a1683a3c89863c9dbedd2b3cc90897" : "-",
    "5bd80f31b2a6d06f6b84e444ed84b744bfebc311" : "-",
    "65272bae7013a785cb4e1dccb810e58e9f7fdfda" : "-",
    "8186bd3db550d2a5cafd840679e8b13ff10a82b5" : "-",
    "87494eab8a1221ccb35e91c14d242e4cfaab28c8" : "-",
    "93eb46dcb67e35e99e3f68b4cb23ee9bf2fbb2d8" : "-",
    "a86dba93c641056fc08ff9dc4bd173bc4b56036f" : "-",
    "b055fc86474cce8854cadadccec61f10dc6ef003" : "-",
    "c05810ec6ce1cb1fb40915b2b16f44c2600c6483" : "-",
    "cf4ee03e4415d599c10729d920a1c085aed411ed" : "-",
    "e20f3bb7c3310deca86cfc66af0a086261930bcf" : "-",

    # ChangeLog edits. Not worth mentioning.
    "0ee6701062d0506a725cd4ba9cd533ec4c46eb68" : "-",
    "197b6a954d37e46af91a474de1565b43e24e8c60" : "-",
    "2027db4808f36bcc230686f31dea3810d594701e" : "-",
    "2583bbaf227abd654cd87760bf6c646909b38229" : "-",
    "388810f1cb7505c7888f6aa77306554b99430202" : "-",
    "6b270891b1b64e26a5c07e9239b925527e7be4c2" : "-",
    "6e99cd9128a04ce8b5daa9b1c5c17c7b86e8a058" : "-",
    "78cff2cb53235979d452b877192523a030686f89" : "-",
    "818367f405a550e95c265d26c7011a5a1f892ec7" : "-",
    "87c7622724089f7a06b6559c9e1103e2f7bac2ef" : "-",
    "925975639a2438bc9bdf43a0ceaab1c4555052b0" : "-",
    "96aaf4408601768d1d277bd63e6a1c91295c4c5a" : "-",
    "9f89d959d4596a5e4918806efc2e5dd044cabc62" : "-",
    "a03abda0f0671ce69b9579b9fcf3d9d499080a5d" : "-",
    "b2efe398cd2a506fe47a6143ea54e08b1f4cbbc7" : "-",
    "b31f72f11b75cf0c7965d8170600e1352988dc25" : "-",
    "b33c4cfbbc49bb6c66e7bdd53d3a2245e1c6a2c9" : "-",
    "b5068c60260646572d3dad166e11c943050b8bd6" : "-",
    "cdd95787ae40cd1ea3776d2322ec529a5a232fc7" : "-",
    "d53da842e2f259282fb83c01ea742d80f7d2735e" : "-",
    "d6552c4c9c10a886ea19768e0097b7344809ea02" : "-",
    "ded2589eae6e392d2ff973d882a0182d2af199cf" : "-",
    "ea8040185053bd2807d0cfdf5cd018f08df10ed1" : "-",
    "ebce3d7682489151a6fc48a923c7154dad401219" : "-",
    "ed6a76513af6a7de5963517e19982293d418ddef" : "-",
    "eeb428b559f482b03ac1002a391a5df3e1512e9a" : "-",
    "f0acc222f33936756297a8240bcfc72c2f9ba891" : "-",

    # Release notes edits. Not worth mentioning.
    "08161bf33c3e17ec81c0d66cb7d2fda678ff6a17" : "-",
    "4f1e0a28875a8c54790d07442bfc3875f903b3c2" : "-",
    "60c7bcbed46bbc266beb642a71018b18c213078e" : "-",
    "8a662850a3ddae4485270c59e9f93c95691f5050" : "-",
    "c34a99c0b18d1122d5613c0048e572d70e7ce751" : "-",

    # Duplicates of other commits. Not worth mentioning.
    "23ab8d114f3038b8425613cd0c74adb160d0bf2f" : "-",
    "3a84d8017df08447b14161cee3c24382f8f96013" : "-",
    "6a7a1881a3fde403e9dacd638d10d457c50ab19e" : "-",
    "d345613880689d38fe6418f47dc19ae855849f92" : "-",

    # Miscellaneous insignificant things.
    "16751a56c0c465f90cb14f65483f967acb1220ae" : "-",
    "25919b3c9601918dbddaccb07ef48fc14a546cf1" : "-",
    "25ee74981392a1f1fde299f8ecf243a6b257e3ae" : "-",
    "5be6de589be48be699d8aa46593d0a0d3e88f46a" : "-",
    "686b994f11a5e7ec2c11e40dc3ca8178c47decdf" : "-",
    "9625d9f3253e0aa4ca8a68380c6303f0b8d36dbe" : "-",
    "a4ecbdbba79e49ce5653101c6c71f8b5df7e0fc5" : "-",
    "e563b107a2b58935ef8c3ab2cfdfdbc2d7cdfa2d" : "-",
    "e7c2d187818158c1c27f87056fba82b0e8077263" : "-",

    # Spammy and uninformative.
    "7f090f7bec5264ca9e203c27dfb6b2992bb2bcbd" : "-",
    "844025196a1b9a5944cac292dbc162bdd24ac5af" : "-",

    # Tsk tsk, McCabe has a potty mouth.
    "7624d729930c331494c7391e3fbc596b31309782" : "-",
    "a67f7ec260b20474cee3c2edca3c3f4ea331c815" : "-",

}



class LogEntry:
    "Stores and formats a Git log entry"

    # A horizontal line to put between log entries
    separator = "-" * 79


    def __init__(self, git_text):
        "Creates a LogEntry from a commit output from `git log`"

        self.id          = ""     # commit id
        self.merge       = False  # is a merge commit?
        self.author_name = ""
        self.author_mail = ""
        self.author_date = ""
        self.commit_name = ""     # committer name
        self.commit_mail = ""
        self.commit_date = ""
        self.message     = []     # lines of the message
        self.files       = []     # modified/added/deleted files
        self.special     = ""     # handle this commit in a special way?

        lines = git_text.splitlines()

        # First line is always commit id.
        self.id = lines[0]

        for line in lines[1:]:

            # is it a merge?
            if re.match("Merge: +([0-9a-f]+ *){2,}", line):
                self.merge = True

            # author name
            match = re.match("Author: +([^<]+) <([^>]+)>", line)
            if match:
                self.author_name = match.group(1)
                self.author_mail = match.group(2)

            # author date
            match = re.match("AuthorDate: +(.+)", line)
            if match:
                self.author_date = match.group(1)

            # commit name
            match = re.match("Commit: +([^<]+) <([^>]+)>", line)
            if match:
                self.commit_name = match.group(1)
                self.commit_mail = match.group(2)

            # commit date
            match = re.match("CommitDate: +(.+)", line)
            if match:
                self.commit_date = match.group(1)

            # message (and notes)
            if re.match("[\t ]+", line):
                self.message.append(line[4:])

            # modified/added/deleted files (but ignore ChangeLog.txt)
            if re.match("[MAD][\t ]+", line) and \
                    not re.match("[MAD][\t ]+ChangeLog.txt", line):
                self.files.append(line)

        self.author_name, self.author_mail = \
            self.fix_name_email( self.author_name, self.author_mail )

        self.commit_name, self.commit_mail = \
            self.fix_name_email( self.commit_name, self.commit_mail )

        try:
            self.special = SPECIAL_COMMITS[self.id]
        except KeyError:
            pass

        # If there's no special already, scan the message for @directives.
        if not self.special:
            for line in self.message:
                # Omit commits with @nochangelog in the message
                if("@nochangelog" in line):
                    self.special = "-"
                # Don't show file changes for commits with @shortchangelog
                elif("@shortchangelog" in line):
                    self.special = "S"
                # Always include commits with @forcechangelog
                elif("@forcechangelog" in line):
                    self.special = "+"


    def fix_name_email( self, name, email ):
        """Some commits have a bad name or email address.
           This function returns the proper name and address."""

        if email in ["Adric@.(none)", "hakushakukun@gmail.com"]:
            return ["McCabe Maxsted", "hakushakukun@gmail.com"]

        if email in ["Kakurady@.(none)", "kakurady@gmail.com"]:
            return ["Kakurady (Geneko Nemeth)", "kakurady@gmail.com"]

        # Nothing to fix.
        return [name, email]


    def should_be_included(self):
        """Returns True if the commit should be included in the
        ChangeLog, or False if it should be omitted."""

        # Include commits marked with "+" or "S" in special_commits.txt
        if self.special in ["+", "S"]:
            return True

        # Omit commits marked with "-" in special_commits.txt
        if self.special == "-":
            return False

        # Omit merge commits
        if self.merge:
            return False

        # Omit vanilla SL source commits
        if self.message[0].startswith("Second Life viewer sources"):
            return False

        # Include everything else
        return True


    def format(self):
        "Formats the LogEntry prettily."
        texts=[LogEntry.separator]

        texts.append("""
Date:       %(ad)s                                                (%(id)s)
Author:     %(an)s  <%(ae)s>""" % { "ad" : self.author_date,
                                    "an" : self.author_name,
                                    "ae" : self.author_mail,
                                    "id" : self.id[0:7] })

        if self.commit_name != self.author_name:
            texts.append("Committer:  %(cn)s  <%(ce)s>" % \
                             { "cn" : self.commit_name,
                               "ce" : self.commit_mail })

        texts.append("\n")

        for line in self.message:
            # Remove @...changelog directives
            rxp = re.compile("@(short|no)changelog")
            if rxp.search(line):
                line = rxp.sub("", line)
                # Skip this line if it was empty except for @...changelog
                if not line.strip():
                    continue

            # Skip modified/deleted/new file lines.
            if re.match("[ \t]+(modified|deleted|new file): +.+", line):
                continue

            # Skip all the rest of the lines.
            if re.match("Conflicts:$", line):
                break

            texts.append( (" "*2 + line.replace("\t"," "*4)).rstrip() )

        # Delete all empty lines at the end
        while not texts[-1].strip():
            del texts[-1]

        # Don't list files if it's a short changelog or there are more
        # than ten files to list.
        if self.special == "S" or len(self.files) > 10:
            texts.append("\n  (File changes omitted for brevity.)")
        else:
            if self.files:
                texts.append("\n")
                for line in self.files:
                    texts.append( " "*2 + line.replace("\t"," "*2) )

        texts.append("\n")

        return "\n".join(texts)



def fail( reason, abort=False ):
    """Prints a message that the ChangeLog couldn't be generated, then
    exits the script. If abort is True, exit with status code 1 (to
    indicate that Make/VisualStudio/Xcode/etc. should abort),
    otherwise exit with status code 0."""

    if abort:
        print "Error: Could not generate ChangeLog.txt: " + reason
        exit(1)
    else:
        print "Warning: Could not generate ChangeLog.txt: " + reason
        exit(0)
        


def main():
    commits = sys.argv[1:]
    if not commits:
        commits = ["HEAD"]


    # Set PATH to help find the git executable on Mac OS X.
    if sys.platform == "darwin":
        os.environ["PATH"] += ":/usr/local/bin:/usr/local/git/bin:/sw/bin:/opt/bin:~/bin"


    # Fetch the log entries from git in one big chunk.
    cmd = ["git", "log", "--pretty=fuller", "--name-status",
           "--date=short", "--date-order"] + commits

    try:
        proc = subprocess.Popen( [" ".join(cmd)], 
                                cwd    = ROOT_DIR,
                                stdout = subprocess.PIPE,
                                stderr = subprocess.STDOUT,
                                shell  = True)
    except OSError:
        fail("The 'git' command is not available.")
    
    output = proc.communicate()[0]
    status = proc.returncode


    # If the git command failed, print the reason and exit.
    if status != 0:
        fail(output)

    # Split it up into individual commits.
    logs = re.compile("^commit ", re.MULTILINE).split(output)[1:]


    # Introductory header that goes at the top of the ChangeLog.
    header="""

                                  CHANGELOG
                           for the Imprudence Viewer
                          http://imprudenceviewer.org


     This is a log of revisions to the Imprudence Viewer source code.
     If you are looking for an overview of new features and changes in
     each release, please see RELEASE_NOTES.txt instead.

     This file is automatically generated from the Git commit history.
     Be aware that it is NOT ORDERED BY DATE, but rather by ancestry.
     Some commits have been omitted from this list for brevity.

     File changes are annotated as follows:

       A  =  the file was added
       M  =  the file was modified
       D  =  the file was deleted

     For a full history of the source code, including diffs, please see
     the Git repository.


"""

    output = [header]

    for log in logs:
        entry = LogEntry(log)
        if entry.should_be_included():
            output.append( entry.format() )

    text = "\n".join(output)

    changelog = open(CHANGELOG, "w")
    changelog.write(text)
    changelog.close()



if __name__ == "__main__":
    main()
