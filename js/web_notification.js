(function(window) {
  if (window.Notification != null) {
    return;
  }
  if (!window.isSecureContext) {
    // Secure context: This feature is available only in secure contexts (HTTPS), in some or all supporting browsers.
    // https://developer.mozilla.org/en-US/docs/Web/API/Notification
    return;
  }

  window._flutter_inappweview_notifications = {};
  window._flutter_inappweview_notification_id_autoincrement = 0;

  class Notification extends EventTarget {
    constructor(title, options) {
      super();
      if (arguments.length === 0) {
        throw TypeError("Failed to construct 'Notification': 1 argument required, but only 0 present.");
      }
      var defaultOptions = {
        actions: [],
        badge: '',
        body: '',
        data: null,
        dir: 'auto',
        icon: '',
        image: null,
        lang: '',
        renotify: false,
        requireInteraction: false,
        silent: false,
        tag: '',
        timestamp: Date.now(),
        vibrate: []
      };
      options = options == null ? defaultOptions : {...defaultOptions, ...options};
      Object.defineProperty(this, "id", {
        value: window._flutter_inappweview_notification_id_autoincrement,
        writable: false,
        enumerable: true,
        configurable: true
      });
      window._flutter_inappweview_notification_id_autoincrement++;
      Object.defineProperty(this, "title", {
        value: title,
        writable: false,
        enumerable: true,
        configurable: true
      });
      this.onclick = null;
      this.onclose = null;
      this.onerror = null;
      this.onshow = null;
      if (Notification.permission === 'granted') {
        window._flutter_inappweview_notifications[this.id] = this;
        if (options != null) {
          for (var key in options) {
            Object.defineProperty(this, key, {
              value: options[key],
              writable: false,
              enumerable: true,
              configurable: true
            });
          }
        }
        var self = this;
        window.flutter_inappwebview.callHandler('Notification.show', {
          id: this.id,
          actions: this.actions,
          badge: this.badge,
          body: this.body,
          data: this.data,
          dir: this.dir,
          icon: this.icon,
          image: this.image,
          lang: this.lang,
          renotify: this.renotify,
          requireInteraction: this.requireInteraction,
          silent: this.silent,
          tag: this.tag,
          timestamp: this.timestamp,
          title: this.title,
          vibrate: this.vibrate
        }).then(function() {
          var event = new Event('show');
          self.dispatchEvent(event);
          if (self.onshow != null) {
            self.onshow(event);
          }
        }).catch(function() {
          var event = new Event('error');
          self.dispatchEvent(event);
          if (self.onerror != null) {
            self.onerror(event);
          }
        });
      }
    }

    close = function() {
      if (window._flutter_inappweview_notifications[this.id] == null) {
        return;
      }
      delete window._flutter_inappweview_notifications[this.id];
      var self = this;
      window.flutter_inappwebview.callHandler('Notification.close', this.id).then(function() {
        var event = new Event('close');
        self.dispatchEvent(event);
        if (self.onclose != null) {
          self.onclose(event);
        }
      }).catch(function() {
        var event = new Event('error');
        self.dispatchEvent(event);
        if (self.onerror != null) {
          self.onerror(event);
        }
      });
    }
  }

  // Static Notification methods
  Notification.requestPermission = function(callback) {
    var self = this;
    return window.flutter_inappwebview.callHandler('Notification.requestPermission').then(function(result) {
      Notification._permission = result;
      if (callback != null) {
        callback(result);
      }
      return result;
    }).catch(function() {
      var event = new Event('error');
      self.dispatchEvent(event);
      if (self.onerror != null) {
        self.onerror(event);
      }
    });
  }

  // Private Notification properties
  Object.defineProperty(Notification, "_permission", {
    value: 'default',
    enumerable: false,
    writable: true
  });

  // Read-only Notification properties
  Object.defineProperty(Notification, "permission", {
    get: function () {
      return Notification._permission;
    },
    set: function () {},
    enumerable: true
  });
  Object.defineProperty(Notification, "maxActions", {
    get: function () {
      return 2;
    },
    set: function () {},
    enumerable: true
  });

  if (window.ServiceWorkerRegistration != null) {
    window.ServiceWorkerRegistration.prototype.showNotification = function(title, options) {
      if (this._notifications == null) {
        this._notifications = [];
      }
      var notifications = this._notifications;
      return new Promise(function(resolve, reject) {
        notifications.push(new Notification(title, options));
        resolve();
      });
    }

    window.ServiceWorkerRegistration.prototype.getNotifications = function(options) {
      var notifications = this._notifications != null ? this._notifications : [];
      if (options != null && options.tag != null) {
        notifications = notifications.filter(function(notification) {
          return notification.tag === options.tag;
        });
      }
      return new Promise(function(resolve, reject) {
        resolve(notifications);
      });
    }
  }

  window.Notification = Notification;
})(window);