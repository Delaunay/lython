

/*
 * Remove syntatic sugar from the ast and generate a canonical/simplifier representation
 *
 * Examples
 * --------
 *
 * Method to functions
 *
 * .. code-block::
 *
 *    class Name:
 *        def __add__(self, a):
 *            return
 *
 *    // data only
 *    class Name:
 *        pass
 *
 *    // function only
 *    def Name#002__add__(self, a):
 *        return
 *
 * .. code-block::
 *
 *    a = Name()
 *    a = Name#001__ctor__()      <= Unique name that cannot clash with
 *                                   Anything in scope
 *
 *  .. code-block::
 *
 *     a + 2
 *     Name#002__add__(a, 2)
 *
 */
class Lowering {};