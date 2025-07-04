use std::cell::Cell;

use crate::{
    abort_now, check_correctness,
    diagnostic::{Diagnostic, Level, SuggestionKind},
};

pub fn abort_if_dirty() {
    check_correctness();
    if IS_DIRTY.with(|c| c.get()) {
        abort_now()
    }
}

pub(crate) fn cleanup() -> Vec<Diagnostic> {
    IS_DIRTY.with(|c| c.set(false));
    vec![]
}

pub(crate) fn emit_diagnostic(diag: Diagnostic) {
    let Diagnostic {
        level,
        span_range,
        msg,
        suggestions,
        children,
    } = diag;

    // On stable, do nothing except set dirty flag for errors.
    if let Level::Error = level {
        IS_DIRTY.with(|c| c.set(true));
    }
    // Suggestions and children are ignored on stable backend.
}

thread_local! {
    static IS_DIRTY: Cell<bool> = Cell::new(false);
}
