# Coding Standards - Phoenix (TypeScript/React)

**Version:** 2.0.0  
**Last Updated:** 2025-01-21  
**Language:** TypeScript 5.0+  
**Framework:** React 18+  
**Build Tool:** Vite  
**Development Environment:** dev-01 (Primary) + Local Machines (Secondary)

---

## Core Principles

### 1. Development Environment First
**ALL CODE CHANGES MUST BE DONE ON DEV-01 FIRST**
- ✅ Build and test on dev-01 before local development
- ✅ Use Tailscale for secure connection to dev-01
- ✅ Test GUI with Xvfb on dev-01
- ✅ Debug resource loading on dev-01
- ✅ Commit changes from dev-01

### 2. User Experience First
Performance, accessibility, and mobile responsiveness are non-negotiable.

### 3. Component Thinking
Build small, reusable components with single responsibility.

### 4. Type Safety
TypeScript everywhere. No `any` types without explicit justification.

### 5. Predictable State
Immutable state updates with clear data flow.

### 6. Ship with Confidence
Test user interactions, not implementation details.

---

## Development Workflow

### **Phase 1: Development on dev-01**
```bash
# Connect to dev-01 via Tailscale
ssh -i ~/.ssh/github_phoenix mark@100.97.54.75
cd /home/ec2-user/workspace/phoenix

# Make code changes
# Test build
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0

# Test GUI with Xvfb
xvfb-run -a ./phoenix_app

# Commit and push
git add .
git commit -m "feat(ui): add new feature"
git push origin feature-branch
```

### **Phase 2: Local Machine Testing**
```bash
# On local machine
git pull origin feature-branch

# Build and test locally
mkdir -p build && cd build
cmake .. -G Ninja
ninja -k0
./phoenix_app
```

### **Key Requirements**
- ✅ **All changes on dev-01 first**
- ✅ **Test with Xvfb on dev-01**
- ✅ **Debug resource loading on dev-01**
- ✅ **Commit from dev-01**
- ✅ **Test locally after sync**

---

## TypeScript/React Standards

### Code Style: Airbnb Guide with Modifications

**Indentation:** 2 spaces
**Line Length:** 100 characters
**Quotes:** Single quotes (except JSX attributes use double)
**Semicolons:** Always use them

```typescript
// Good
const userName = 'Alice';
const message = `Hello, ${userName}`;

// JSX attributes use double quotes
<Button label="Click me" />

// Bad
const userName = "Alice"  // No semicolon, double quotes
```

---

### Naming Conventions

**Components and Types:** `PascalCase`
```typescript
interface UserProfile {
  name: string;
  email: string;
}

function UserCard({ name, email }: UserProfile) {
  return <div>{name}</div>;
}
```

**Functions, Variables, Hooks:** `camelCase`
```typescript
const userName = 'Alice';
const calculateTotal = (items: Item[]) => sum(items);

// Custom hooks start with 'use'
function useUserData(userId: string) {
  const [user, setUser] = useState<User | null>(null);
  return { user, setUser };
}
```

**Constants:** `UPPER_SNAKE_CASE`
```typescript
const MAX_RETRY_ATTEMPTS = 3;
const API_BASE_URL = 'https://api.example.com';
const DEFAULT_TIMEOUT_MS = 5000;
```

**Event Handlers:** `handle*` prefix
```typescript
function LoginForm() {
  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    // ...
  };

  const handleInputChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    // ...
  };

  return <form onSubmit={handleSubmit}>...</form>;
}
```

**Boolean Variables:** `is*`, `has*`, `should*` prefix
```typescript
const isLoading = true;
const hasPermission = checkPermission();
const shouldShowModal = isLoading && hasPermission;
```

---

### Component Structure

**Order matters:**
```typescript
// 1. Imports
import React, { useState, useEffect } from 'react';
import { fetchUserData } from '@/api/users';
import { Button } from '@/components/ui/Button';
import styles from './UserProfile.module.css';

// 2. Types/Interfaces
interface UserProfileProps {
  userId: string;
  onUpdate?: (user: User) => void;
}

interface User {
  id: string;
  name: string;
  email: string;
}

// 3. Constants
const REFRESH_INTERVAL_MS = 30000;

// 4. Component
export function UserProfile({ userId, onUpdate }: UserProfileProps) {
  // 4a. Hooks
  const [user, setUser] = useState<User | null>(null);
  const [isLoading, setIsLoading] = useState(true);

  // 4b. Effects
  useEffect(() => {
    fetchUserData(userId).then(setUser);
  }, [userId]);

  // 4c. Event handlers
  const handleRefresh = () => {
    setIsLoading(true);
    fetchUserData(userId).then(data => {
      setUser(data);
      setIsLoading(false);
      onUpdate?.(data);
    });
  };

  // 4d. Early returns
  if (isLoading) return <LoadingSpinner />;
  if (!user) return <ErrorMessage />;

  // 4e. Render
  return (
    <div className={styles.container}>
      <h2>{user.name}</h2>
      <p>{user.email}</p>
      <Button onClick={handleRefresh}>Refresh</Button>
    </div>
  );
}
```

---

### TypeScript Usage

**Always use TypeScript, never JavaScript:**
```typescript
// Good
interface ButtonProps {
  label: string;
  onClick: () => void;
  variant?: 'primary' | 'secondary';
  disabled?: boolean;
}

function Button({ label, onClick, variant = 'primary', disabled = false }: ButtonProps) {
  return (
    <button 
      onClick={onClick} 
      disabled={disabled}
      className={`btn-${variant}`}
    >
      {label}
    </button>
  );
}

// Bad - no types
function Button({ label, onClick, variant, disabled }) {
  return <button onClick={onClick}>{label}</button>;
}
```

**Never use `any` without justification:**
```typescript
// Bad
const data: any = await fetchData();

// Good
interface ApiResponse {
  data: User[];
  total: number;
}

const response: ApiResponse = await fetchData();

// Acceptable - with explicit justification
// eslint-disable-next-line @typescript-eslint/no-explicit-any
const legacyData: any = parseOldFormat(data); // TODO: Type this properly
```

**Use proper prop interfaces:**
```typescript
// Good
interface ChatMessageProps {
  message: {
    id: string;
    content: string;
    timestamp: Date;
    author: string;
  };
  onDelete: (id: string) => void;
  className?: string;
}

// Bad
interface ChatMessageProps {
  message: any;
  onDelete: Function;
}
```

---

## Hooks Best Practices

### Custom Hooks

**Return objects, not arrays (unless it's a tuple):**
```typescript
// Good - object return
function useUserData(userId: string) {
  const [user, setUser] = useState<User | null>(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState<Error | null>(null);

  return { user, loading, error, refetch: () => fetchUser(userId) };
}

// Good - tuple return (like useState)
function useToggle(initialValue = false): [boolean, () => void] {
  const [value, setValue] = useState(initialValue);
  const toggle = () => setValue(v => !v);
  return [value, toggle];
}

// Bad - array return with many items
function useUserData(userId) {
  return [user, loading, error, refetch, isStale, lastUpdate];
}
```

### useEffect Cleanup

**Always cleanup subscriptions:**
```typescript
// Good
useEffect(() => {
  const subscription = dataStream.subscribe(handleData);
  
  return () => {
    subscription.unsubscribe();
  };
}, []);

// Bad - memory leak
useEffect(() => {
  dataStream.subscribe(handleData);
}, []);
```

### useMemo for Expensive Calculations

```typescript
// Good
const sortedItems = useMemo(() => {
  return items.sort((a, b) => a.price - b.price);
}, [items]);

// Bad - recalculates every render
const sortedItems = items.sort((a, b) => a.price - b.price);
```

### useCallback to Prevent Re-renders

```typescript
// Good - memoized callback
function ParentComponent() {
  const handleClick = useCallback((id: string) => {
    console.log('Clicked:', id);
  }, []);

  return <ChildComponent onClick={handleClick} />;
}

// Bad - new function every render
function ParentComponent() {
  return <ChildComponent onClick={(id) => console.log('Clicked:', id)} />;
}
```

---

## Testing Standards

### React Testing Library (Not Enzyme)

**Test user behavior, not implementation:**
```typescript
// Good
import { render, screen, fireEvent } from '@testing-library/react';

test('displays user name after clicking load button', async () => {
  render(<UserProfile userId="123" />);
  
  const loadButton = screen.getByRole('button', { name: /load/i });
  fireEvent.click(loadButton);
  
  const userName = await screen.findByText(/alice/i);
  expect(userName).toBeInTheDocument();
});

// Bad - testing implementation
test('sets loading state to true when fetching', () => {
  const { result } = renderHook(() => useUserData('123'));
  
  act(() => {
    result.current.fetch();
  });
  
  expect(result.current.loading).toBe(true);
});
```

### Coverage Requirements

**Minimum:** 75% overall
**Critical components:** >90% (authentication, payment, forms)

### Mock API Calls

```typescript
// Good
import { rest } from 'msw';
import { setupServer } from 'msw/node';

const server = setupServer(
  rest.get('/api/users/:id', (req, res, ctx) => {
    return res(ctx.json({ id: req.params.id, name: 'Alice' }));
  })
);

beforeAll(() => server.listen());
afterEach(() => server.resetHandlers());
afterAll(() => server.close());
```

---

## Styling Standards

### CSS Modules for Complex Components

```typescript
// UserCard.module.css
.container {
  padding: 1rem;
  border-radius: 8px;
}

.title {
  font-size: 1.5rem;
  font-weight: bold;
}

// UserCard.tsx
import styles from './UserCard.module.css';

function UserCard() {
  return (
    <div className={styles.container}>
      <h2 className={styles.title}>User Name</h2>
    </div>
  );
}
```

### Tailwind for Utility-Based Styling

```typescript
// Good - utility classes
<div className="p-4 rounded-lg bg-white shadow-md hover:shadow-lg transition-shadow">
  <h2 className="text-xl font-bold text-gray-900">Title</h2>
</div>

// Combine with conditional classes
<button 
  className={`
    px-4 py-2 rounded 
    ${isPrimary ? 'bg-blue-500 text-white' : 'bg-gray-200 text-gray-800'}
    ${disabled ? 'opacity-50 cursor-not-allowed' : 'hover:opacity-90'}
  `}
>
  Click Me
</button>
```

### No Inline Styles for Layout

```typescript
// Bad
<div style={{ padding: '20px', marginTop: '10px' }}>Content</div>

// Good
<div className="p-5 mt-2.5">Content</div>

// Acceptable - dynamic styles
<div style={{ width: `${percentage}%` }}>Progress</div>
```

### Mobile-First Responsive Design

```css
/* Good - mobile first */
.container {
  padding: 1rem;
}

@media (min-width: 768px) {
  .container {
    padding: 2rem;
  }
}

/* Bad - desktop first */
.container {
  padding: 2rem;
}

@media (max-width: 767px) {
  .container {
    padding: 1rem;
  }
}
```

---

## Performance Standards

### Lazy Load Routes

```typescript
// Good
import { lazy, Suspense } from 'react';

const Dashboard = lazy(() => import('./pages/Dashboard'));
const Settings = lazy(() => import('./pages/Settings'));

function App() {
  return (
    <Suspense fallback={<LoadingSpinner />}>
      <Routes>
        <Route path="/dashboard" element={<Dashboard />} />
        <Route path="/settings" element={<Settings />} />
      </Routes>
    </Suspense>
  );
}
```

### Code Splitting for Large Components

```typescript
// Good
const HeavyChart = lazy(() => import('./components/HeavyChart'));

function Dashboard() {
  const [showChart, setShowChart] = useState(false);

  return (
    <div>
      <button onClick={() => setShowChart(true)}>Show Chart</button>
      {showChart && (
        <Suspense fallback={<div>Loading chart...</div>}>
          <HeavyChart />
        </Suspense>
      )}
    </div>
  );
}
```

### Use Context to Avoid Prop Drilling

```typescript
// Good
const ThemeContext = createContext<Theme | null>(null);

function App() {
  const [theme, setTheme] = useState<Theme>('light');
  
  return (
    <ThemeContext.Provider value={{ theme, setTheme }}>
      <Layout />
    </ThemeContext.Provider>
  );
}

function DeepNestedComponent() {
  const { theme } = useContext(ThemeContext);
  return <div className={theme}>Content</div>;
}
```

---

## Accessibility Standards (WCAG 2.1 AA)

### Semantic HTML

```typescript
// Good
<nav>
  <ul>
    <li><a href="/home">Home</a></li>
  </ul>
</nav>

<button onClick={handleClick}>Submit</button>

// Bad
<div onClick={handleClick}>Submit</div>
```

### Proper ARIA Labels

```typescript
// Good
<button 
  aria-label="Close dialog"
  onClick={onClose}
>
  <XIcon />
</button>

<img src="profile.jpg" alt="User profile picture" />

// Bad
<button onClick={onClose}>
  <XIcon />
</button>

<img src="profile.jpg" />
```

### Keyboard Navigation

```typescript
// Good
function DropdownMenu() {
  const handleKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Escape') {
      closeMenu();
    }
  };

  return (
    <div 
      role="menu" 
      onKeyDown={handleKeyDown}
      tabIndex={0}
    >
      Menu items
    </div>
  );
}
```

### Form Labels

```typescript
// Good
<label htmlFor="email">Email address</label>
<input 
  id="email" 
  type="email" 
  aria-required="true"
  aria-describedby="email-error"
/>
<span id="email-error" role="alert">{error}</span>

// Bad
<input type="email" placeholder="Email" />
```

---

## Anti-Patterns to Avoid

### 1. Mutating State Directly

```typescript
// Bad
function TodoList() {
  const [todos, setTodos] = useState<Todo[]>([]);

  const addTodo = (text: string) => {
    todos.push({ id: Date.now(), text }); // Mutation!
    setTodos(todos);
  };
}

// Good
function TodoList() {
  const [todos, setTodos] = useState<Todo[]>([]);

  const addTodo = (text: string) => {
    setTodos([...todos, { id: Date.now(), text }]);
  };
}
```

### 2. Using Index as Key

```typescript
// Bad
{items.map((item, index) => (
  <li key={index}>{item.name}</li>
))}

// Good
{items.map(item => (
  <li key={item.id}>{item.name}</li>
))}
```

### 3. useEffect for Derived Values

```typescript
// Bad
function UserProfile({ user }: { user: User }) {
  const [displayName, setDisplayName] = useState('');

  useEffect(() => {
    setDisplayName(`${user.firstName} ${user.lastName}`);
  }, [user]);

  return <div>{displayName}</div>;
}

// Good
function UserProfile({ user }: { user: User }) {
  const displayName = `${user.firstName} ${user.lastName}`;
  return <div>{displayName}</div>;
}
```

### 4. Async in useEffect Without Cleanup

```typescript
// Bad
useEffect(() => {
  async function fetchData() {
    const data = await api.getUser();
    setUser(data);
  }
  fetchData();
}, []);

// Good
useEffect(() => {
  let cancelled = false;

  async function fetchData() {
    const data = await api.getUser();
    if (!cancelled) {
      setUser(data);
    }
  }

  fetchData();

  return () => {
    cancelled = true;
  };
}, []);
```

---

## Git Commit Standards

**Format:** `<type>(<scope>): <subject>`

**Examples:**
```bash
feat(chat): add message streaming
fix(auth): correct token refresh logic
style(button): update hover states
refactor(api): extract common error handling
test(chat): add integration tests
docs(readme): update setup instructions
```

---

## Pre-Commit Checklist

Before submitting a PR:

- [ ] **TypeScript compiles:** `npm run type-check`
- [ ] **ESLint passes:** `npm run lint`
- [ ] **Prettier formatted:** `npm run format`
- [ ] **Tests pass:** `npm run test`
- [ ] **No console.log:** Remove debug statements
- [ ] **Accessibility checked:** Test with keyboard only
- [ ] **Responsive:** Test on mobile, tablet, desktop
- [ ] **Performance:** Check bundle size, loading time
- [ ] **Types complete:** No `any` types without justification

---

## Commands

```bash
# Type checking
npm run type-check

# Linting
npm run lint
npm run lint:fix

# Formatting
npm run format
npm run format:check

# Testing
npm run test
npm run test:watch
npm run test:coverage

# Build
npm run build

# Full pre-commit
npm run type-check && npm run lint && npm run test
```

---

## Version History

### 2.1.0 (2025-01-23)
- **MAJOR UPDATE**: Added comprehensive CI workflow protection
- **NEW**: Self-healing CI system with conflict prevention
- **NEW**: Pre-commit hooks for conflict marker detection
- **NEW**: YAML linting and workflow validation
- **NEW**: CODEOWNERS protection for workflow changes
- **NEW**: Weekly self-audit cron triggers
- **UPDATED**: CI workflow documentation and standards

### 2.0.0 (2025-01-21)
- **MAJOR UPDATE**: Added dev-01-first development policy
- **NEW**: Tailscale integration for secure development
- **NEW**: Xvfb GUI testing on dev-01
- **NEW**: Resource debugging workflow
- **UPDATED**: Development workflow requirements
- **UPDATED**: Quality assurance standards

### 1.0.0 (2025-01-14)
- Initial TypeScript/React coding standards
- Component structure guidelines
- Hooks best practices
- Accessibility requirements
- Testing standards

---

**Maintained By:** Engineering Team  
**Questions:** See `.underlord/docs/GOTCHAS.md` for common issues
