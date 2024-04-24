def validate_type(key, input, expected_type):
    """
    Validate type of the dictionary input.
    input:
        key: key of the dictionary
        input: dictionary input
        expected_type: type to expect.
    example:
        validate_type(key, my_dict[key], float)
    """
    if not isinstance(input, expected_type):
        raise TypeError('INPUT ERROR: ' + key + ' expects type ' + repr(expected_type) + ' -- got %s' % type(input).__name__)

def key_exists(dictionary, key):
    try:
        dictionary[key]
        return True
    except KeyError:
        return False
