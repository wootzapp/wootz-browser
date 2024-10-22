package com.wootz.browser;

import java.lang.annotation.ElementType;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.lang.annotation.Target;

/**
 * Marks a method as being able to be exposed to JavaScript.
 *
 * This is used for safety purposes. Otherwise, the only reasonable behavior
 * would be to expose all public methods to JavaScript.
 *
 * @see {@link WootzView#addJavascriptInterface(Object, String)}
 */
@SuppressWarnings("javadoc")
@Retention(RetentionPolicy.RUNTIME)
@Target({ElementType.METHOD})
public @interface WootzJavascriptInterface {

}
