/**
 * For our purposes, an enumeration is a fixed set of CSS-expression-compatible
 * names. When serialized, a selected subset of the members may be specified as
 * whitespace-separated strings. An enumeration deserializer is a function that
 * parses a serialized subset of an enumeration and returns any members that are
 * found as a Set.
 *
 * The following example will produce a deserializer for the days of the
 * week:
 *
 * const deserializeDaysOfTheWeek = enumerationDeserializer([
 *   'Monday',
 *   'Tuesday',
 *   'Wednesday',
 *   'Thursday',
 *   'Friday',
 *   'Saturday',
 *   'Sunday'
 * ]);
 */
export declare const enumerationDeserializer: <T extends string>(allowedNames: T[]) => (valueString: string) => Set<T>;
