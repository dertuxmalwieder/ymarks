/**
 * Copyright 2017 Google Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
const path = require('path');
const helper = require('./helper');

class ElementHandle {
  /**
   * @param {!Connection} client
   * @param {!Object} remoteObject
   * @param {!Mouse} mouse
   * @param {!Touchscreen} touchscreen;
   */
  constructor(client, remoteObject, mouse, touchscreen) {
    this._client = client;
    this._remoteObject = remoteObject;
    this._mouse = mouse;
    this._touchscreen = touchscreen;
    this._disposed = false;
  }

  /* async */ dispose() {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    if (this._disposed)
      return;
    this._disposed = true;
    (yield helper.releaseObject(this._client, this._remoteObject));
  });}

  /**
   * @param {function()} pageFunction
   * @param {!Array<*>} args
   * @return {!Promise<(!Object|undefined)>}
   */
  /* async */ evaluate(pageFunction, ...args) {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    console.assert(!this._disposed, 'ElementHandle is disposed!');
    console.assert(typeof pageFunction === 'function', 'First argument to ElementHandle.evaluate must be a function!');

    const stringifiedArgs = ['this'];
    stringifiedArgs.push(...args.map(x => JSON.stringify(x)));
    const functionDeclaration = `function() { return (${pageFunction})(${stringifiedArgs.join(',')}) }`;
    const objectId = this._remoteObject.objectId;
    const { exceptionDetails, result: remoteObject } = (yield this._client.send('Runtime.callFunctionOn', { objectId, functionDeclaration, returnByValue: false, awaitPromise: true}));
    if (exceptionDetails)
      throw new Error('Evaluation failed: ' + helper.getExceptionMessage(exceptionDetails));
    return (yield helper.serializeRemoteObject(this._client, remoteObject));
  });}

  /**
   * @return {!Promise<{x: number, y: number}>}
   */
  /* async */ _visibleCenter() {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    const center = (yield this.evaluate(element => {
      if (!element.ownerDocument.contains(element))
        return null;
      element.scrollIntoViewIfNeeded();
      const rect = element.getBoundingClientRect();
      return {
        x: (Math.max(rect.left, 0) + Math.min(rect.right, window.innerWidth)) / 2,
        y: (Math.max(rect.top, 0) + Math.min(rect.bottom, window.innerHeight)) / 2
      };
    }));
    if (!center)
      throw new Error('No node found for selector: ' + selector);
    return center;
  });}

  /* async */ hover() {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    const {x, y} = (yield this._visibleCenter());
    (yield this._mouse.move(x, y));
  });}

  /**
   * @param {!Object=} options
   */
  /* async */ click(options) {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    const {x, y} = (yield this._visibleCenter());
    (yield this._mouse.click(x, y, options));
  });}

  /**
   * @param {!Array<string>} filePaths
   * @return {!Promise}
   */
  /* async */ uploadFile(...filePaths) {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    const files = filePaths.map(filePath => path.resolve(filePath));
    const objectId = this._remoteObject.objectId;
    return this._client.send('DOM.setFileInputFiles', { objectId, files });
  });}

  /* async */ tap() {return (fn => {
  const gen = fn.call(this);
  return new Promise((resolve, reject) => {
    function step(key, arg) {
      let info, value;
      try {
        info = gen[key](arg);
        value = info.value;
      } catch (error) {
        reject(error);
        return;
      }
      if (info.done) {
        resolve(value);
      } else {
        return Promise.resolve(value).then(
            value => {
              step('next', value);
            },
            err => {
              step('throw', err);
            });
      }
    }
    return step('next');
  });
})(function*(){
    const {x, y} = (yield this._visibleCenter());
    (yield this._touchscreen.tap(x, y));
  });}
}

module.exports = ElementHandle;
helper.tracePublicAPI(ElementHandle);
