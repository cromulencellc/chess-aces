## Morse

Morse is an injection vulnerability into the open source NodeBB project version 1.19.5. NodeBB is an open-source NodeJS webserver providing a forum. It uses mongodb for storage. https://community.nodebb.org/


### Running

Run command with access to shell (node user)
1. `docker-compose run --rm --service-ports ta3_morse bash`
2. `./nodebb setup`
3. `./nodebb start`

### Poller

The poller runs several common tasks for nodebb. It runs a random number of actions and picks random actions by random users. 

There is a cheatsheet/poller_cheatsheet.py containing the usernames and passwords for accounts on the database, knowledge of or contained in these files must not be assumed or known by any performers. 

The poller will modify the forum each run as it writes new posts and creates groups. These actions do not alter the ability of subsequent pollers nor of the POVs. However, the forums base_data with standard user accounts/data can be reset by replacing the data in the base_data folders with those from the backup_base_data folder.

##### Base Data Reset

In the event the mongodb database needs to be refreshed, the challenge/base_data folder should be replaced with the backup_base_data folder. This will return the data to the original condition for the challenge. The challenge will need to be rebuilt.

The mongodb container does not automatically exit when the challenge/ref_patch containers are stopped. It will need to be manually stopped before the data can be reset to the original state - the base_data is a volume in the mongodb container.

### PoV 1

* CWE-22/41: Path Traversal

Effect:     Unstructured Privileged Information Disclosure

This POV shows an attacker can access the *config.json* file located in the */usr/src/app/* directory. The config file contains the session secret that is used to sign and verify cookies.

This weakness allows an attacker to read a *.json* file and extract values by knowing a key. The path of a language file is not sanitized when accessed via translation tags `[]`. Additionally, NodeBB generates HTML meta tags (specficially `og:url`) based on translation `[]` that use the language files. Allowing an attacker to perform a path traversal and read the contents of a key/value pair from any json file. The resulting information is present in the source for the page.

### PoV 2

* CWE-269/285: Access Control
* CWE-287: Authentication Error

Effect:     Authentication Bypass

This POV shows an attacker can change any user's password by using that user's UID in the change password request, instead of their own. Specifically, the admin user's password is changed from the chess user's account.

The change password function incorrectly uses the uid value instead of the data.uid value. This value is user/attacker supplied. Whereas, the data.uid value is associated with the user who is performing the request. This allows an attacker who is logged in to change the password for any user by selecting the UID associated with that user.

### PoV 3

* CWE-74: Data/Code Injection
* CWE-79: Cross-site Scripting (stored)

Effect:     Unstructured Privileged Information Disclosure

This POV relies on the path traversal in POV1. Instead of an arbitrary read there is a write primative that allows injecting code from an atttacker into a victim's browser (Stored-XSS), which allows cookie stealing.

The POV shows an attacker altering their profile's *aboutme* field to contain javascript. The profile is exported using socket.io, which stores the profile in a JSON file on the server in the *build/export/* folder. The attacker profile's *aboutme* field is then altered to contain a translation with path traversal (POV1) - `[[../../../export/uid_profile.json]]`, where uid is the attacker's uid (unique identifier #). If another user accesses the attacker's profile page, then the script is activated. The javascipt embedded in the profile connects to an attacker controlled server, which receives a connection from the victim with cookie attached. The cookie is unstructed privileged information.

The POV performs all parts of the attack: attacker profile changes, attacker server waiting for victim and logging cookie, and the victim accessing the attacker's profile after changes. This POV requires additional complexity through the means of a headless chrome browser, implemented by puppeteer in js.

Iterative operations of this pov are unlikely to succeed, the base_data will need to be reset.

### PoV 4

* CWE-400/405/703: Resource Exhaustion(Heap) via Logic Error, assisted with amplification through rate limit bypass

Effect:     Denial of Service due to Resource Exhaustion(Heap)

To allow this POV to succeed in a reasonable amount of time, the heap memory allocated for the challenge has been limited to 1GB. This is noted in the Dockerfile `ENV NODE_OPTIONS='--max-old-space-size=1024'`.

The *uploads.upload* function allows a user to upload a cover photo or profile picture. The function incorrectly uses a cache storage, which is never garbage collected if a download isn't finished. Therefore, with enough iterations of uploads the heap memory can be exhausted.

This POV takes about 2.5 minutes to succeed. However, by monitoring top on the challenge container, one can determine if the memory usage is increasing. In the event memory use does not substantially increase, then it can be assumed the pov has failed. The POV relys on the rate limit bypass (discussed next) - this can also cause the pov to finish, but not succeed. In this event the challenge will have a message `warn: Flooding detected! Calls : ###, Duration : ####`. In this case, the POV is considered failed.

Rate limit bypass:<br />
NodeBB has a check against a user flooding the server through the socket.io interface. This check does not prevent a command starting with `admin`(note no period). However, if the input command does not start with `admin.` (note the period) but does have admin at the start, then admin is removed from the start. The flooding check is still satified as it uses the original unedited version of the string. This allows an attacker to perform operations without limit.

Failure for this POV will occur when all requests have been serviced or the timeout (240 seconds) is reached, signifying that is unlikely leaking memory and did not cause a crash. To verify ensure the memory usage by the challenge returns to normal(lower) after the POV stops running, or never increases substantially while the POV is running.