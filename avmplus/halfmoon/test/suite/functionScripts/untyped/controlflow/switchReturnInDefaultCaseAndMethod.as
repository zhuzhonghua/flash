// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

function switchTest() {
	var x = 10;

    switch (x) {
        case 10:
            return 50;
        default:
        return 30;
    }

    return 100;
}

print(switchTest());
