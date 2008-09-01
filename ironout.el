(defgroup ironout nil
  "ironout, A C refactoring tool."
  :link '(url-link "http://ironout.berlios.de/")
  :prefix "ironout-")

(defcustom ironout-path "ironout"
  "The path to ironout executable.")

(defun ironout-find ()
  "Find occurrences of a name."
  (interactive)
  (let ((buffer (get-buffer-create "*ironout-find*")))
    (with-current-buffer buffer
      (setq buffer-read-only 'nil)
      (erase-buffer))
    (call-process ironout-path
		  nil buffer t
		  "find" (buffer-file-name) (int-to-string
					     (- (+ (point-min) (point))
						1)))
    (with-current-buffer buffer
      (goto-char (point-min))
      (setq buffer-read-only 't)
      (local-set-key (kbd "RET") 'ironout-goto-occurrence)
      (local-set-key (kbd "C-j") 'ironout-goto-occurrence)
      (local-set-key (kbd "q") 'delete-window))
    (setq next-error-function 'ironout-goto-next)
    (display-buffer buffer)
  ))

(defun ironout-goto-next (arg reset)
  (display-buffer "*ironout-find*")
  (with-current-buffer "*ironout-find*"
    (if reset
	(goto-char (point-min)))
    (forward-line (- arg 1))
    (if (eobp)
	(message "Moved past last occurrence!")
      (ironout-goto-occurrence))
    (forward-line 1))
  )

(defun ironout-goto-occurrence ()
  (interactive)
  (with-current-buffer "*ironout-find*"
    (end-of-line)
    (let ((end (point)))
      (beginning-of-line)
      (let ((line (buffer-substring-no-properties (point) end)))
	(let ((tokens (split-string line)))
	  (find-file (nth 0 tokens))
	  (goto-char (+ (string-to-number (nth 1 tokens)) 1))
	  )
	))))

(defvar ironout-local-keymap (make-sparse-keymap))

(define-minor-mode ironout-mode
 "ironout, a C refactoring tool!" nil " Iron" ironout-local-keymap
  :global nil)

(define-key ironout-local-keymap (kbd "C-c f") 'ironout-find)
(add-hook 'c-mode-hook 'ironout-mode)
(provide 'ironout)
