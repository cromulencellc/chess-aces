/* forked from https://github.com/VRMink/credit-card-generator
 * licensed under the GNU GPL
 * swapped visa prefixes and amex prefixes to make the numbers still
 * luhn but not map to real card numbers (visa and amex have different
 * lengths)
 */

var pseudoRandom = Math.random;

var amexPrefixList = new Array(
    "4539",
    "4556",
    "4916",
    "4532",
    "4929",
    "40240071",
    "4485",
    "4716",
    "4"
);

var visaPrefixList = new Array(
    "34",
    "37"
);

/**
 * Revert a String
 * @param  {String} str
 * @return {String}
 */
function strrev(str) {
   if (!str) return '';
   var revstr='';
   for (var i = str.length-1; i>=0; i--)
       revstr+=str.charAt(i)
   return revstr;
}

/**
 * Complete a prefixed number-string
 * @param  {String} prefix  is the start of the CC number as a string, any number of digits
 * @param  {Number} length  is the length of the CC number to generate. Typically 13 or 16
 * @return {String}
 */
function completed_number(prefix, length) {

    var ccnumber = prefix;

    // generate digits

    while ( ccnumber.length < (length - 1) ) {
        ccnumber += Math.floor(pseudoRandom()*10);
    }

    // reverse number and convert to int

    var reversedCCnumberString = strrev( ccnumber );

    var reversedCCnumber = new Array();
    for ( var i=0; i < reversedCCnumberString.length; i++ ) {
        reversedCCnumber[i] = parseInt( reversedCCnumberString.charAt(i) );
    }

    // calculate sum

    var sum = 0;
    var pos = 0;

    while ( pos < length - 1 ) {

        var odd = reversedCCnumber[ pos ] * 2;
        if ( odd > 9 ) {
            odd -= 9;
        }

        sum += odd;

        if ( pos != (length - 2) ) {

            sum += reversedCCnumber[ pos +1 ];
        }
        pos += 2;
    }

    // calculate check digit

    var checkdigit = (( Math.floor(sum/10) + 1) * 10 - sum) % 10;
    ccnumber += checkdigit;

    return ccnumber;

}

/**
 * Actually generate a credit card number
 * @param  {[type]} prefixList [description]
 * @param  {[type]} length     [description]
 * @param  {[type]} howMany    [description]
 * @return {[type]}            [description]
 */
function credit_card_number(prefixList, length, howMany) {

    var result = new Array();
    for (var i = 0; i < howMany; i++) {

        var randomArrayIndex = Math.floor(pseudoRandom() * prefixList.length);
        var ccnumber = prefixList[ randomArrayIndex ];
        result.push( completed_number(ccnumber, length) );
    }

    return result;
}

/**
 * Supported Card Schemes
 * @type {Array}
 */
module.exports.Schemes = {
    "VISA": {
        prefixList: visaPrefixList,
        digitCount: 16
    },
    "Amex": {
        prefixList: amexPrefixList,
        digitCount: 15
    }
}

/**
 * The entry-point function
 * @param {String} CardScheme  The Card Scheme
 * @param {Number} [howMany]   Defaults to 1
 * @param {Number} [randomGen] Pseudo Random Generator. Must generate a random number between 0 an 1
 * @return {String}
 */
module.exports.GenCC = function(CardScheme, howMany, randomGen){
    pseudoRandom = randomGen || pseudoRandom;
    var amount = howMany || 1;
    // Try to get configs to the selected Scheme
    if (typeof module.exports.Schemes[CardScheme] != 'undefined') {
        return credit_card_number(
            module.exports.Schemes[CardScheme].prefixList,
            module.exports.Schemes[CardScheme].digitCount,
            amount
        );
    }
    else { // Defaults to MasterCard
        return credit_card_number(
            module.exports.Schemes["Amex"].prefixList,
            module.exports.Schemes["Amex"].digitCount,
            amount
        );
    }
}

console.log(module.exports.GenCC('Amex', 5, pseudoRandom));
console.log(module.exports.GenCC('VISA', 5, pseudoRandom));
